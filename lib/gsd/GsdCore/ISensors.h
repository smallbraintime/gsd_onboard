#pragma once

#include <etl/optional.h>

namespace gsd {
struct Geo {
    int32_t latitude;
    int32_t longitude;
    int32_t altitude;
    int32_t velocity;
    uint32_t cog;
};

class ISensors {
   public:
    virtual ~ISensors() = default;
    virtual etl::optional<Geo> getGeo() = 0;
    virtual int8_t getBatteryPercentage() = 0;
    virtual bool isOk() = 0;
};
}  // namespace gsd
