#pragma once

extern "C" {
#include <mavlink/v2.0/common/mavlink.h>
}
#include <etl/vector.h>
#include <cstdint>

namespace gsd {
using MavPacket = etl::vector<uint8_t, MAVLINK_MAX_PACKET_LEN>;

class IMavSocket {
   public:
    virtual ~IMavSocket() {};
    virtual void write(const MavPacket& packet) = 0;
    virtual bool read(MavPacket& packet) = 0;
};
}  // namespace gsd