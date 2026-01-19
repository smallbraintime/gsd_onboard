#include "Sensors.h"

Sensors::Sensors(int8_t gpsRxPin,
                 int8_t batteryRxPin,
                 int32_t batteryMaxMv,
                 int32_t batteryMinMv,
                 float voltageDivider)
    : _gpsPin(gpsRxPin),
      _batteryPin(batteryRxPin),
      _batteryMaxMv(batteryMaxMv),
      _batteryMinMv(batteryMinMv),
      _batteryRangeMv(_batteryMaxMv - _batteryMinMv),
      _voltageDivider(voltageDivider) {}

void Sensors::begin() {
    _gpsSerial.begin(9600, SERIAL_8N1, _gpsPin, -1);

    pinMode(_batteryPin, INPUT);
    analogReadResolution(12);
    analogSetPinAttenuation(_batteryPin, ADC_11db);

    _isGpsOk = true;
    _isBatteryOk = true;
}

void Sensors::end() {
    _gpsSerial.end();
    _isBatteryOk = false;
    _isBatteryOk = false;
}

etl::optional<gsd::Gps> Sensors::getGps() {
    while (_gpsSerial.available() > 0) {
        if (_gps.encode(_gpsSerial.read())) {
            GSD_DEBUG("speed: %i", _gps.speed.kmph());

            gsd::Gps gps{};

            if (_gps.location.isValid()) {
                int32_t lat = static_cast<int32_t>(_gps.location.rawLat().deg) * 10000000 +
                              static_cast<int32_t>(_gps.location.rawLat().billionths / 100);
                gps.latitude = _gps.location.rawLat().negative ? -lat : lat;

                int32_t lon = static_cast<int32_t>(_gps.location.rawLng().deg) * 10000000 +
                              static_cast<int32_t>(_gps.location.rawLng().billionths / 100);
                gps.longitude = _gps.location.rawLng().negative ? -lon : lon;

                gps.altitude = static_cast<int32_t>(_gps.altitude.value() * 10);
            }

            if (_gps.speed.isValid())
                gps.velocity = static_cast<int32_t>(_gps.speed.value()) * 5144 / 1000;

            if (_gps.course.isValid())
                gps.cog = static_cast<uint32_t>(_gps.course.value());

            GSD_DEBUG("%i %i", gps.latitude, gps.longitude);
            return gps;
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }

    return etl::nullopt;
};

int8_t Sensors::getBatteryPercentage() {
    if (_batteryPin == -1) {
        _isBatteryOk = false;
        return -1;
    }

    int32_t pinMv = analogReadMilliVolts(_batteryPin);
    int32_t batMv = static_cast<int32_t>(_voltageDivider * static_cast<float>(pinMv));
    int32_t offsetMv = batMv - _batteryMinMv;
    int32_t percentage = (offsetMv * 100) / _batteryRangeMv;

    if (pinMv < 50)
        _isBatteryOk = false;
    else
        _isBatteryOk = true;

    return static_cast<int8_t>(constrain(percentage, 0, 100));
}

bool Sensors::isBatteryOk() {
    return _isBatteryOk;
}

bool Sensors::isGpsOk() {
    return _isGpsOk;
}