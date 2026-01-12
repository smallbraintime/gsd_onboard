#pragma once

extern "C" {
#include <mavlink/v2.0/common/mavlink.h>
}
#include <etl/atomic.h>
#include <etl/mutex.h>
#include <etl/optional.h>
#include <etl/type_traits.h>
#include <cstdint>

#include "Debug.h"
#include "IDrive.h"
#include "IMavSocket.h"
#include "ISecurityKeyProvider.h"
#include "ISensors.h"
#include "ITimer.h"
#include "IVideoStream.h"
#include "MavPacketProvider.h"

namespace gsd {
template <typename T>
class MavlinkGateway {
    ETL_STATIC_ASSERT(etl::is_base_of<ITimer, T>::value);

   public:
    struct Config {
        uint32_t heartbeatTxIntervalMs = 1000;
        uint32_t connectionTimeoutMs = 15000;
        uint8_t sysId = 1;
        uint8_t compId = 1;
        bool msgSigning = false;  // TODO: implement signing
    };

    enum class VehicleState : uint8_t {
        Unknown,
        Ok,
        Critical,
        Emergency,
    };

    MavlinkGateway(const MavlinkGateway&) = delete;
    MavlinkGateway& operator=(const MavlinkGateway&) = delete;

    MavlinkGateway(const Config& config,
                   IMavSocket* socket,
                   ISensors* sensors,
                   IVideoStream& videoStream,
                   IDrive& drive,
                   ISecurityKeyProvider& keyProvider)
        : _config(config),
          _socket(socket),
          _sensors(sensors),
          _videoStream(videoStream),
          _drive(drive),
          _keyProvider(keyProvider),
          _packetProvider(config.sysId, config.sysId),
          _heartbeatTxContext{_socket, &_packetProvider, &_mutex, &_vehicleState},
          _dataTxContext{_socket, &_packetProvider, &_mutex, _sensors, &_vehicleState} {
        GSD_ASSERT(socket);
        GSD_ASSERT(sensors);
    }

    void update() {
        if (_socket->read(_packet))
            parseMavPacket(_packet);

        if (!_isConnected.load()) {
            _videoStream.stop();
            _dataTxTimer.detach();
            _heartbeatTxTimer.detach();
        }
    }

    bool isConnected() const { return _isConnected; }

    VehicleState vehicleState() const { return _vehicleState; }

   private:
    struct HeartbeatTxContext {
        IMavSocket* socket;
        MavPacketProvider* packetProvider;
        etl::mutex* mutex;
        etl::atomic<VehicleState>* vehicleState;
    };

    struct DataTxContext {
        IMavSocket* socket;
        MavPacketProvider* packetProvider;
        etl::mutex* mutex;
        ISensors* sensors;
        etl::atomic<VehicleState>* vehicleState;
    };

    static void sendHeartbeat(HeartbeatTxContext* ctx) {
        uint8_t systemState;

        switch (ctx->vehicleState->load()) {
            case VehicleState::Unknown:
                systemState = MAV_STATE_UNINIT;
                break;
            case VehicleState::Ok:
                systemState = MAV_STATE_ACTIVE;
                break;
            case VehicleState::Critical:
                systemState = MAV_STATE_CRITICAL;
                break;
            case VehicleState::Emergency:
                systemState = MAV_STATE_EMERGENCY;
                break;
            default:
                GSD_ASSERT(false);
        }

        ctx->mutex->lock();
        ctx->socket->write(ctx->packetProvider->heartbeat(systemState));
        ctx->mutex->unlock();
    }

    static void sendData(DataTxContext* ctx) {
        VehicleState newState = VehicleState::Ok;

        etl::optional<Geo> geoOpt = ctx->sensors->getGeo();
        if (geoOpt) {
            Geo& geo = geoOpt.value();

            ctx->mutex->lock();
            ctx->socket->write(ctx->packetProvider->gpsRaw(geo.latitude, geo.longitude,
                                                           geo.altitude, geo.velocity, geo.cog));
            ctx->mutex->unlock();
        } else {
            if (ctx->vehicleState->load() > VehicleState::Critical)
                newState = VehicleState::Critical;
        }

        int8_t batPerc = ctx->sensors->getBatteryPercentage();
        if (batPerc != -1) {
            ctx->mutex->lock();
            ctx->socket->write(ctx->packetProvider->batteryStatus(batPerc));
            ctx->mutex->unlock();
        } else {
            newState = VehicleState::Emergency;
        }

        *ctx->vehicleState = newState;
    }

    static void handleConnectionTimeout(etl::atomic<bool>* isConnected) { *isConnected = false; }

    void parseMavPacket(const MavPacket& packet) {
        mavlink_message_t msg;
        mavlink_status_t status;

        for (size_t i = 0; i < packet.size(); ++i) {
            if (mavlink_parse_char(MAVLINK_COMM_0, packet[i], &msg, &status))
                processMavlinkMessage(msg);
        }

        if (status.packet_rx_success_count <= 0)
            GSD_LOG("failed to parse mavlink packet");
    }

    void processMavlinkMessage(const mavlink_message_t& msg) {
        _isConnected = true;

        if (!_heartbeatTxTimer.active())
            _heartbeatTxTimer.attachMs(_config.heartbeatTxIntervalMs, sendHeartbeat,
                                       &_heartbeatTxContext);
        _connectionTimeoutTimer.onceMs(_config.connectionTimeoutMs, handleConnectionTimeout,
                                       &_isConnected);

        VehicleState newState = VehicleState::Ok;

        switch (msg.msgid) {
            case MAVLINK_MSG_ID_HEARTBEAT:
                mavlink_heartbeat_t heartbeat;
                mavlink_msg_heartbeat_decode(&msg, &heartbeat);
                break;
            case MAVLINK_MSG_ID_REQUEST_DATA_STREAM: {
                mavlink_request_data_stream_t request;
                mavlink_msg_request_data_stream_decode(&msg, &request);

                if (!request.start_stop) {
                    _dataTxTimer.detach();
                    break;
                }

                auto rateMs = static_cast<uint32_t>(1000 / request.req_message_rate);
                if (!_dataTxTimer.active())
                    _dataTxTimer.attachMs(rateMs, sendData, &_dataTxContext);

                IVideoStream::Url url = _videoStream.start();
                if (url.empty()) {
                    newState = VehicleState::Critical;
                } else {
                    _mutex.lock();
                    _socket->write(_packetProvider.videoStreamInfo(url.c_str()));
                    _mutex.unlock();
                }
            } break;
            case MAVLINK_MSG_ID_MANUAL_CONTROL: {
                mavlink_manual_control_t manualControl;
                mavlink_msg_manual_control_decode(&msg, &manualControl);

                if (!_drive.move(manualControl.x, manualControl.r))
                    newState = VehicleState::Emergency;
            } break;
            case MAVLINK_MSG_ID_WIFI_CONFIG_AP: {
                mavlink_wifi_config_ap_t wifiConfig;
                mavlink_msg_wifi_config_ap_decode(&msg, &wifiConfig);

                _mutex.lock();
                _socket->changePassword(wifiConfig.ssid, wifiConfig.password);
                _mutex.unlock();
            } break;
            default:
                GSD_LOG("unsupported mavlink message type");
        }

        _vehicleState = newState;
    }

    const Config _config;
    etl::atomic<bool> _isConnected = false;
    IMavSocket* const _socket;
    ISensors* const _sensors;
    IVideoStream& _videoStream;
    IDrive& _drive;
    ISecurityKeyProvider& _keyProvider;
    MavPacketProvider _packetProvider;
    etl::atomic<VehicleState> _vehicleState = VehicleState::Unknown;
    etl::mutex _mutex;
    HeartbeatTxContext _heartbeatTxContext;
    DataTxContext _dataTxContext;
    T _heartbeatTxTimer;
    T _dataTxTimer;
    T _connectionTimeoutTimer;
    MavPacket _packet;
};
}  // namespace gsd