#pragma once

extern "C" {
#include <mavlink/v2.0/common/mavlink.h>
}
#include <etl/optional.h>
#include <etl/type_traits.h>
#include <cstdint>

#include "Debug.h"
#include "IDrive.h"
#include "IMavSocket.h"
#include "INotifier.h"
#include "ISecurityKeyProvider.h"
#include "ISensors.h"
#include "IVideoStream.h"
#include "MavPacketProvider.h"

namespace gsd {
template <class T>
class MavlinkGateway {
    ETL_STATIC_ASSERT(etl::is_base_of<INotifier, T>::value);

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

    explicit MavlinkGateway(const Config& config,
                            IMavSocket& socket,
                            ISensors& sensors,
                            IVideoStream& videoStream,
                            IDrive& drive,
                            ISecurityKeyProvider& keyProvider)
        : _config(config),
          _socket(socket),
          _sensors(sensors),
          _videoStream(videoStream),
          _drive(drive),
          _keyProvider(keyProvider),
          _packetProvider(config.sysId, config.sysId) {}

    void update() {
        if (_socket.read(_packet))
            parseMavPacket(_packet);

        if (_shouldSendHeartbeat.checkAndClear()) {
            resolveVehicleState();
            sendHeartbeat();
        }

        if (_shouldSendData.checkAndClear()) {
            sendData();
        }

        if (_disconnected.check()) {
            _videoStream.stop();
            _dataTxTimer.stop();
            _heartbeatTxTimer.stop();
            _videoStreamRequested.clear();
        }
    }

    bool isConnected() const { return !_disconnected.check(); }

    VehicleState vehicleState() const { return _vehicleState; }

   private:
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
        _disconnected.clear();

        _heartbeatTxTimer.notifyEvery(_config.heartbeatTxIntervalMs, &_shouldSendHeartbeat);
        _connectionTimeoutTimer.notifyOnce(_config.connectionTimeoutMs, &_disconnected);

        switch (msg.msgid) {
            case MAVLINK_MSG_ID_HEARTBEAT:
                mavlink_heartbeat_t heartbeat;
                mavlink_msg_heartbeat_decode(&msg, &heartbeat);
                break;
            case MAVLINK_MSG_ID_REQUEST_DATA_STREAM: {
                mavlink_request_data_stream_t request;
                mavlink_msg_request_data_stream_decode(&msg, &request);

                if (!request.start_stop) {
                    _dataTxTimer.stop();
                    _videoStream.stop();
                    _videoStreamRequested.clear();
                    break;
                }

                auto rateMs = static_cast<uint32_t>(1000 / request.req_message_rate);
                _dataTxTimer.notifyEvery(rateMs, &_shouldSendData);

                _videoStreamRequested.notify();
            } break;
            case MAVLINK_MSG_ID_MANUAL_CONTROL: {
                mavlink_manual_control_t manualControl;
                mavlink_msg_manual_control_decode(&msg, &manualControl);

                _drive.move(manualControl.x, manualControl.r);
            } break;
            case MAVLINK_MSG_ID_WIFI_CONFIG_AP: {
                mavlink_wifi_config_ap_t wifiConfig;
                mavlink_msg_wifi_config_ap_decode(&msg, &wifiConfig);

                _socket.changePassword(wifiConfig.ssid, wifiConfig.password);
            } break;
            default:
                GSD_LOG("unsupported mavlink message type");
        }
    }

    void sendHeartbeat() {
        uint8_t systemState;

        switch (_vehicleState) {
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

        _socket.write(_packetProvider.heartbeat(systemState));
    }

    void sendData() {
        etl::optional<Geo> geoOpt = _sensors.getGeo();
        if (geoOpt) {
            Geo& geo = geoOpt.value();

            _socket.write(_packetProvider.gpsRaw(geo.latitude, geo.longitude, geo.altitude,
                                                 geo.velocity, geo.cog));
        }

        int8_t batPerc = _sensors.getBatteryPercentage();
        if (batPerc != -1)
            _socket.write(_packetProvider.batteryStatus(batPerc));
    }

    void resolveVehicleState() {
        if (!_drive.isOk()) {
            _vehicleState = VehicleState::Emergency;
        } else if (_videoStreamRequested && !_videoStream.isStreaming()) {
            IVideoStream::Url url = _videoStream.start();
            if (!url.empty())
                _socket.write(_packetProvider.videoStreamInfo(url.c_str()));

            if (_videoStream.isStreaming())
                _vehicleState = VehicleState::Critical;
        } else if (!_sensors.isOk()) {
            _vehicleState = VehicleState::Critical;
        } else {
            _vehicleState = VehicleState::Ok;
        }
    }

    const Config _config;
    IMavSocket& _socket;
    ISensors& _sensors;
    IVideoStream& _videoStream;
    IDrive& _drive;
    ISecurityKeyProvider& _keyProvider;
    MavPacketProvider _packetProvider;
    VehicleState _vehicleState = VehicleState::Unknown;
    Condition _disconnected{true};
    Condition _shouldSendData;
    Condition _shouldSendHeartbeat;
    Condition _videoStreamRequested;
    T _heartbeatTxTimer;
    T _dataTxTimer;
    T _connectionTimeoutTimer;
    MavPacket _packet;
};
}  // namespace gsd