#pragma once

#include <cstdint>

extern "C" {
#include <mavlink/v2.0/common/mavlink.h>
}
#include "IMavSocket.h"

namespace gsd {
class MavPacketProvider {
   public:
    explicit MavPacketProvider(uint8_t sysId, uint8_t compId) : _sysId(sysId), _compId(compId) {}

    const MavPacket& heartbeat(uint8_t systemStatus);
    const MavPacket& gpsRaw(int32_t lat, int32_t lon, int32_t alt, uint16_t cog, uint16_t vel);
    const MavPacket& batteryStatus(int8_t batteryRemaining);
    const MavPacket& videoStreamInfo(const char* url);

   private:
    mavlink_message_t _message;
    MavPacket _buffer;
    const uint8_t _sysId, _compId;
};
}  // namespace gsd