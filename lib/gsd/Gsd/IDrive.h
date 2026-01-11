#pragma once

#include <cstdint>

namespace gsd {
class IDrive {
   public:
    virtual ~IDrive() = default;
    virtual bool move(int16_t forward, int16_t yaw) = 0;
};
}  // namespace gsd