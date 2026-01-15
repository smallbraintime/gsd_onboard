#pragma once

#include <Ticker.h>
#include <etl/atomic.h>

#include <GsdCore/ITicker.h>

class GsdTicker : public gsd::ITicker {
   public:
    GsdTicker(const GsdTicker&) = delete;
    GsdTicker& operator=(const GsdTicker&) = delete;

    GsdTicker() = default;

    void start(uint32_t ms) override;
    void stop() override;
    bool ticked() override;
    bool active() override;

   private:
    static void tick(etl::atomic<bool>* hasTicked);

    Ticker _ticker;
    etl::atomic<bool> _hasTicked = false;
};
