#pragma once

#include <Ticker.h>

#include <GsdCore/INotifier.h>

class Notifier : public gsd::INotifier {
   public:
    Notifier(const Notifier&) = delete;
    Notifier& operator=(const Notifier&) = delete;

    Notifier() = default;

    void notifyEvery(uint32_t ms, gsd::Condition* condition) {
        _ticker.attach_ms(ms, notify, condition);
    }
    void notifyOnce(uint32_t ms, gsd::Condition* condition) {
        _ticker.once_ms(ms, notify, condition);
    };
    void stop() { _ticker.detach(); };

   private:
    static void notify(gsd::Condition* condition) { condition->notify(); }

    Ticker _ticker;
};
