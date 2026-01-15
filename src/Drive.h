#pragma once

#include "DShotESC.h"

#include <GsdCore/IDrive.h>

#include "Debug.h"

class Drive : public gsd::IDrive {
   public:
    Drive(const Drive&) = delete;
    Drive& operator=(const Drive&) = delete;

    Drive() = default;

    bool begin(int8_t leftEscTxPin, int8_t rightEscTxPin) { return true; }
    void stop() {}
    void move(int16_t forward, int16_t yaw) {
    }  // TODO: map to dshot and delegate to a separate task
    bool isOk() { return _isOk; };

   private:
    DShotESC _leftEsc;
    DShotESC _rightEsc;
    bool _isOk = false;
};