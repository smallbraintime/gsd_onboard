#pragma once

#include <optional>

namespace gsd {
class ISensors {
   public:
    struct Imu {};
    struct Geo {};

    ~ISensors() = default;
    virtual bool init() = 0;
    virtual std::optional<Imu> getImu() = 0;
    virtual std::optional<Geo> getGeo() = 0;
    virtual int getBatteryLevel() = 0;
};
}  // namespace gsd
