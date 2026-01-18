#include "MavPacketProvider.h"

namespace gsd {
const MavPacket& MavPacketProvider::heartbeat(uint8_t systemStatus) {
    _buffer.resize(MAVLINK_MAX_PACKET_LEN);

    mavlink_msg_heartbeat_pack(_sysId, MAV_COMP_ID_PERIPHERAL, &_message, MAV_TYPE_GENERIC,
                               MAV_AUTOPILOT_GENERIC, 0, 0, systemStatus);

    const uint16_t len = mavlink_msg_to_send_buffer(_buffer.data(), &_message);
    _buffer.resize(static_cast<size_t>(len));

    return _buffer;
}

const MavPacket& MavPacketProvider::gpsRaw(int32_t lat,
                                           int32_t lon,
                                           int32_t alt,
                                           uint16_t vel,
                                           uint16_t cog) {
    _buffer.resize(MAVLINK_MAX_PACKET_LEN);

    mavlink_msg_gps_raw_int_pack(_sysId, _compId, &_message, 0, 0, lat, lon, alt, 0, 0, vel, cog, 0,
                                 0, 0, 0, 0, 0, 0);

    const uint16_t len = mavlink_msg_to_send_buffer(_buffer.data(), &_message);
    _buffer.resize(static_cast<size_t>(len));

    return _buffer;
}

const MavPacket& MavPacketProvider::batteryStatus(int8_t batteryRemaining) {
    _buffer.resize(MAVLINK_MAX_PACKET_LEN);

    mavlink_msg_battery_status_pack(_sysId, _compId, &_message, 0, 0, 0, 0, nullptr, 0, 0, 0,
                                    batteryRemaining, 0, 0, nullptr, 0, 0);

    const uint16_t len = mavlink_msg_to_send_buffer(_buffer.data(), &_message);
    _buffer.resize(static_cast<size_t>(len));

    return _buffer;
}

const MavPacket& MavPacketProvider::videoStreamInfo(const char* url) {
    _buffer.resize(MAVLINK_MAX_PACKET_LEN);

    mavlink_msg_video_stream_information_pack(_sysId, _compId, &_message, 0, 1, 0, 0, 0, 0, 0, 0, 0,
                                              0, nullptr, url, 0, 0);

    const uint16_t len = mavlink_msg_to_send_buffer(_buffer.data(), &_message);
    _buffer.resize(static_cast<size_t>(len));

    return _buffer;
}
}  // namespace gsd
