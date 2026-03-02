#pragma once

#include <etl/atomic.h>

#include <GsdCore/IDrive.h>

#include "Debug.h"

class Drive : public gsd::IDrive {
   public:
    Drive(const Drive&) = delete;
    Drive& operator=(const Drive&) = delete;

    Drive(int8_t d0, int8_t d1, int8_t d2, int8_t d3);

    void begin();
    void end();
    void move(int16_t forward, int16_t yaw) override;
    bool isOk() override;

   private:
    etl::atomic<int16_t> _lastForward = 0;
    etl::atomic<int16_t> _lastYaw = 0;
    volatile bool _isOk = false;
    const int8_t _d0;
    const int8_t _d1;
    const int8_t _d2;
    const int8_t _d3;
    const uint32_t _frequency = 20000;
    const uint8_t _resolution = 8;
};