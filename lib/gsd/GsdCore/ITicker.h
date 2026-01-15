#pragma once

#include <cstdint>

namespace gsd {
class ITicker {
   public:
    ~ITicker() = default;
    virtual void start(uint32_t ms) = 0;
    virtual void stop() = 0;
    virtual bool ticked() = 0;
    virtual bool active() = 0;
};
}  // namespace gsd