#pragma once

#include "GsdCore/ISensors.h"

class Sensors : public gsd::ISensors {
   public:
    Sensors(const Sensors&) = delete;
    Sensors& operator=(const Sensors&) = delete;

    Sensors() = default;

    etl::optional<gsd::Geo> getGeo() { return gsd::Geo{}; };
    int8_t getBatteryPercentage() { return 0; };
    uint64_t getTimestamp() { return {}; };
    bool isOk() { return true; };
};