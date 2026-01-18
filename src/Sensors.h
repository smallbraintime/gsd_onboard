#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>
#include <Preferences.h>
#include <TinyGPS++.h>

#include <GsdCore/ISensors.h>

#include "Debug.h"

class Sensors : public gsd::ISensors {
   public:
    Sensors(const Sensors&) = delete;
    Sensors& operator=(const Sensors&) = delete;

    Sensors(int32_t batteryMaxMv, int32_t batteryMinMv, float voltageDivider);

    void begin(int8_t gpsRxPin, int8_t batteryRxPin);
    void stop();
    etl::optional<gsd::Gps> getGps() override;
    int8_t getBatteryPercentage() override;
    bool isBatteryOk() override;
    bool isGpsOk() override;

   private:
    TinyGPSPlus _gps;
    HardwareSerial _gpsSerial{1};
    int8_t _batPin = -1;
    int32_t _batteryMaxMv;
    int32_t _batteryMinMv;
    int32_t _batteryRangeMv;
    float _voltageDivider;
    bool _isGpsOk = false;
    bool _isBatteryOk = false;
};