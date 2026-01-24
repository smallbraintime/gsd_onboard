#pragma once

extern "C" {
#include <mavlink/v2.0/common/mavlink.h>
}
#include <etl/optional.h>
#include <etl/type_traits.h>
#include <cstdint>

#include "IDrive.h"
#include "IMavSocket.h"
#include "ISecurity.h"
#include "ISensors.h"
#include "ITicker.h"
#include "IVideoStream.h"
#include "MavPacketProvider.h"

namespace gsd {
template <class T>
class MavlinkGateway {
    ETL_STATIC_ASSERT(etl::is_base_of<ITicker, T>::value);

   public:
    struct Config {
        uint32_t heartbeatTxIntervalMs = 1000;
        uint8_t sysId = 1;
        uint8_t compId = 1;
        bool msgSigning = false;  // TODO: implement signing
    };

    MavlinkGateway(const MavlinkGateway&) = delete;
    MavlinkGateway& operator=(const MavlinkGateway&) = delete;

    explicit MavlinkGateway(const Config& config,
                            IMavSocket& socket,
                            ISensors& sensors,
                            IVideoStream& videoStream,
                            IDrive& drive,
                            ISecurity& security)
        : _config(config),
          _socket(socket),
          _sensors(sensors),
          _videoStream(videoStream),
          _drive(drive),
          _security(security),
          _packetProvider(config.sysId, config.compId) {}

    void update() {
        processIncoming();
        sendPeriodic();

        if (!_socket.peerAlive()) {
            _videoStream.stop();
            _dataTxTicker.stop();
            _heartbeatTxTicker.stop();
        }
    }

   private:
    void sendPeriodic() {
        if (_heartbeatTxTicker.active() && _heartbeatTxTicker.ticked())
            sendHeartbeat();

        if (_dataTxTicker.active() && _dataTxTicker.ticked())
            sendData();
    }

    void processIncoming() {
        if (_socket.read(_packet)) {
            mavlink_message_t msg;
            mavlink_status_t status;

            for (size_t i = 0; i < _packet.size(); ++i) {
                if (mavlink_parse_char(MAVLINK_COMM_0, _packet[i], &msg, &status))
                    processMavlinkMessage(msg);
            }
        }
    }

    void processMavlinkMessage(const mavlink_message_t& msg) {
        if (!_heartbeatTxTicker.active())
            _heartbeatTxTicker.start(_config.heartbeatTxIntervalMs);

        switch (msg.msgid) {
            case MAVLINK_MSG_ID_HEARTBEAT:
                mavlink_heartbeat_t heartbeat;
                mavlink_msg_heartbeat_decode(&msg, &heartbeat);
                break;
            case MAVLINK_MSG_ID_REQUEST_DATA_STREAM: {
                mavlink_request_data_stream_t request;
                mavlink_msg_request_data_stream_decode(&msg, &request);

                auto rateMs = static_cast<uint32_t>(1000 / request.req_message_rate);
                if (!_dataTxTicker.active())
                    _dataTxTicker.start(rateMs);

                _url = _videoStream.start();
                if (!_url.empty())
                    _socket.write(_packetProvider.videoStreamInfo(_url.c_str()));
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
        }
    }

    void sendHeartbeat() {
        uint8_t systemState;

        if (!_drive.isOk() || !_sensors.isBatteryOk() ||
            (!_videoStream.isOk() && !_sensors.isGpsOk()))
            systemState = MAV_STATE_EMERGENCY;
        else if (!_videoStream.isOk() || !_sensors.isGpsOk())
            systemState = MAV_STATE_CRITICAL;
        else
            systemState = MAV_STATE_ACTIVE;

        _socket.write(_packetProvider.heartbeat(systemState));
    }

    void sendData() {
        etl::optional<Gps> gpsOpt = _sensors.getGps();
        Gps gps{};
        if (gpsOpt) {
            gps = gpsOpt.value();
        }
        _socket.write(_packetProvider.gpsRaw(gps.latitude, gps.longitude, gps.altitude,
                                             gps.velocity, gps.cog));

        int8_t batPerc = _sensors.getBatteryPercentage();
        _socket.write(_packetProvider.batteryStatus((batPerc != -1) ? batPerc : 0));
    }

    const Config _config;
    IMavSocket& _socket;
    ISensors& _sensors;
    IVideoStream& _videoStream;
    IDrive& _drive;
    ISecurity& _security;
    MavPacketProvider _packetProvider;
    T _heartbeatTxTicker;
    T _dataTxTicker;
    MavPacket _packet;
    IVideoStream::Url _url;
};
}  // namespace gsd