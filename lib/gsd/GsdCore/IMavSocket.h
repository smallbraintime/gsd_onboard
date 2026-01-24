#pragma once

extern "C" {
#include <mavlink/v2.0/common/mavlink.h>
}
#include <etl/array.h>
#include <etl/delegate.h>
#include <etl/vector.h>
#include <cstdint>

namespace gsd {
using MavPacket = etl::vector<uint8_t, MAVLINK_MAX_PACKET_LEN>;

class IMavSocket {
   public:
    using Callback = etl::delegate<void(MavPacket&)>;

    virtual ~IMavSocket() = default;
    virtual void write(const MavPacket& packet, bool discreet = false) = 0;
    virtual bool read(MavPacket& packet) = 0;
    // virtual void setReadCallback(Callback callback) = 0;
    virtual bool peerAlive() = 0;
    virtual void changePassword(const char* oldPassword, const char* newPassword) = 0;
};
}  // namespace gsd