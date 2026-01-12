#pragma once

#include "GsdCore/IDrive.h"

class Drive : public gsd::IDrive {
   public:
    void move(int16_t forward, int16_t yaw) {};
    bool isOk() { return true; };
};