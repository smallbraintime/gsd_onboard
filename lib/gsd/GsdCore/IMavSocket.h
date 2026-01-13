#pragma once

extern "C" {
#include <mavlink/v2.0/common/mavlink.h>
}
#include <etl/array.h>
#include <etl/vector.h>
#include <cstdint>

namespace gsd {
using MavPacket = etl::vector<uint8_t, MAVLINK_MAX_PACKET_LEN>;

class IMavSocket {
   public:
    using Key = etl::array<uint8_t, 32>;

    virtual ~IMavSocket() = default;
    virtual void write(const MavPacket& packet) = 0;
    virtual bool read(MavPacket& packet) = 0;
    virtual void changePassword(const char* oldPassword, const char* newPassword) = 0;
    virtual void setLowTxPower() = 0;
    virtual void setHighTxPower() = 0;
    virtual Key getSecretKey() = 0;
};
}  // namespace gsd