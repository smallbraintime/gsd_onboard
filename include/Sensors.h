#pragma once

#include <optional>

namespace gsd {
class Sensors {
   public:
    struct Imu {};
    struct Geo {};

    Sensors(const Sensors&) = delete;
    Sensors& operator=(const Sensors&) = delete;

    Sensors() = default;
    void init();
    std::optional<Imu> getImu();
    std::optional<Geo> getGeo();
    int getBatteryLevel();
};
}  // namespace gsd