#pragma once

namespace gsd {
class Actuators {
   public:
    Actuators(const Actuators&) = delete;
    Actuators& operator=(const Actuators&) = delete;

    Actuators() = default;
    
    void init();
    void setControl(float throttle, float roll, float pitch, float yaw);
};
}  // namespace gsd