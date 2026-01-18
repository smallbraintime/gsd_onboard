#pragma once

#include <etl/optional.h>

namespace gsd {
struct Gps {
    int32_t latitude = 0;
    int32_t longitude = 0;
    int32_t altitude = 0;
    int32_t velocity = 0;
    uint32_t cog = 0;
};

class ISensors {
   public:
    virtual ~ISensors() = default;
    virtual etl::optional<Gps> getGps() = 0;
    virtual int8_t getBatteryPercentage() = 0;
    virtual bool isBatteryOk() = 0;
    virtual bool isGpsOk() = 0;
};
}  // namespace gsd
