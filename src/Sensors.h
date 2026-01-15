#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>

#include <GsdCore/ISensors.h>

#include "Debug.h"

class Sensors : public gsd::ISensors {
   public:
    Sensors(const Sensors&) = delete;
    Sensors& operator=(const Sensors&) = delete;

    Sensors() = default;

    bool begin(int8_t gpsRxPin, int8_t gpsTxPin, int8_t batteryTxPin) { return true; }
    void stop() {}
    etl::optional<gsd::Geo> getGeo() override { return etl::nullopt; };
    int8_t getBatteryPercentage() override { return 0; };
    bool isOk() override { return _isOk; }

   private:
    SFE_UBLOX_GNSS _gnss;
    Preferences _preferences;
    bool _isOk = false;
    uint64_t _timestamp;
};