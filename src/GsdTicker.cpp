#include "GsdTicker.h"

void GsdTicker::start(uint32_t ms) {
    _ticker.attach_ms(ms, tick, &_hasTicked);
}

void GsdTicker::stop() {
    _ticker.detach();
}

bool GsdTicker::ticked() {
    return _hasTicked.exchange(false, etl::memory_order_relaxed);
}

bool GsdTicker::active() {
    return _ticker.active();
}

void GsdTicker::tick(etl::atomic<bool>* hasTicked) {
    hasTicked->store(true, etl::memory_order_relaxed);
}