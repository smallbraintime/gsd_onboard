#pragma once

#include <cstdint>

namespace gsd {
class ITimer {
   public:
    template <typename T>
    void attachMs(uint32_t ms, void (*func)(T*), T* arg) {
        attachMsInternal(ms, reinterpret_cast<void (*)(void*)>(func), static_cast<void*>(arg));
    }

    template <typename T>
    void onceMs(uint32_t ms, void (*func)(T*), T* arg) {
        onceMsInternal(ms, reinterpret_cast<void (*)(void*)>(func), static_cast<void*>(arg));
    }

    virtual void detach() = 0;
    virtual bool active() = 0;

   protected:
    virtual void attachMsInternal(uint32_t ms, void (*func)(void*), void* arg) = 0;
    virtual void onceMsInternal(uint32_t ms, void (*func)(void*), void* arg) = 0;
};
}  // namespace gsd