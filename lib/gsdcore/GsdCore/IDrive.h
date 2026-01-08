#pragma once

namespace gsd {
class IDrive {
   public:
    ~IDrive() = default;
    virtual bool init() = 0;
    virtual void move(float forward, float yaw) = 0;
};
}  // namespace gsd