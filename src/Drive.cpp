#include "Drive.h"

Drive::Drive(int8_t d0, int8_t d1, int8_t d2, int8_t d3) : _d0(d0), _d1(d1), _d2(d2), _d3(d3) {}

void Drive::begin() {
    if (ledcSetup(0, _frequency, _resolution) && ledcSetup(1, _frequency, _resolution) &&
        ledcSetup(2, _frequency, _resolution) && ledcSetup(3, _frequency, _resolution))
        _isOk = true;
    else
        GSD_DEBUG("failed to init ledc");

    ledcAttachPin(_d0, 0);
    ledcAttachPin(_d1, 1);
    ledcAttachPin(_d2, 2);
    ledcAttachPin(_d3, 3);

    ledcWrite(0, 0);
    ledcWrite(1, 0);
    ledcWrite(2, 0);
    ledcWrite(3, 0);
}

void Drive::end() {
    ledcDetachPin(_d0);
    ledcDetachPin(_d1);
    ledcDetachPin(_d2);
    ledcDetachPin(_d3);
}

void Drive::move(int16_t forward, int16_t yaw) {
    if (forward != _lastForward.load() || yaw != _lastYaw.load()) {
        const int16_t left = constrain(forward + yaw, -1000, 1000);
        const int16_t right = constrain(forward - yaw, -1000, 1000);

        if (left >= 0) {
            ledcWrite(1, 0);
            ledcWrite(0, map(left, 0, 1000, 0, 255));
        } else {
            ledcWrite(1, map(abs(left), 0, 1000, 0, 255));
            ledcWrite(0, 0);
        }

        if (right >= 0) {
            ledcWrite(3, 0);
            ledcWrite(2, map(right, 0, 1000, 0, 255));
        } else {
            ledcWrite(3, map(abs(right), 0, 1000, 0, 255));
            ledcWrite(2, 0);
        }
    }

    _lastForward = forward;
    _lastYaw = yaw;
}

bool Drive::isOk() {
    return _isOk;
};