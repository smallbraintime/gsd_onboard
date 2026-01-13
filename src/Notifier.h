#pragma once

#include <Ticker.h>

#include <GsdCore/INotifier.h>

class Notifier : public gsd::INotifier {
   public:
    Notifier(const Notifier&) = delete;
    Notifier& operator=(const Notifier&) = delete;

    Notifier() = default;

    void notifyEvery(uint32_t ms, gsd::Condition* condition) override;
    void notifyOnce(uint32_t ms, gsd::Condition* condition) override;
    void stop() override;

   private:
    static void notify(gsd::Condition* condition);

    Ticker _ticker;
};
