#pragma once

#include <etl/atomic.h>
#include <cstdint>

namespace gsd {
class Condition {
   public:
    explicit Condition(bool flag = false) : _flag(flag) {}

    void notify() { _flag.store(true, etl::memory_order_relaxed); }
    void clear() { _flag.store(false, etl::memory_order_relaxed); }
    bool checkAndClear() { return _flag.exchange(false, etl::memory_order_relaxed); }
    bool check() const { return _flag.load(); }
    operator bool() const { return _flag.load(); }

   private:
    etl::atomic<bool> _flag;
};

class INotifier {
   public:
    ~INotifier() = default;
    virtual void notifyEvery(uint32_t ms, Condition* condition) = 0;
    virtual void notifyOnce(uint32_t ms, Condition* condition) = 0;
    virtual void stop() = 0;
};
}  // namespace gsd