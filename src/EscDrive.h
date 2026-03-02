#pragma once

#include <DShotESC.h>
#include <etl/atomic.h>

#include <GsdCore/IDrive.h>

#include "Debug.h"
#include "DriveHistory.h"

class EscDrive : public gsd::IDrive {
   public:
    EscDrive(const EscDrive&) = delete;
    EscDrive& operator=(const EscDrive&) = delete;

    EscDrive(int8_t leftEscTxPin, int8_t rightEscTxPin);

    void begin();
    void end();
    void move(int16_t forward, int16_t yaw) override;
    bool isOk() override;

   private:
    static void motorHandler(void* pvParameters);

    TaskHandle_t _taskHandle = NULL;
    etl::atomic<int16_t> _forward = 0;
    etl::atomic<int16_t> _yaw = 0;
    volatile bool _isOk = false;
    volatile bool _running = false;
    volatile bool _isRecovering = false;
    const int8_t _leftEscPin;
    const int8_t _rightEscPin;
    DriveHistory<1000, 2> _driveHistory{2};
};