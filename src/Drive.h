#pragma once

#include "GsdCore/IDrive.h"

class Drive : public gsd::IDrive {
   public:
    Drive(const Drive&) = delete;
    Drive& operator=(const Drive&) = delete;

    Drive() = default;

    void move(int16_t forward, int16_t yaw) {};
    bool isOk() { return true; };
};