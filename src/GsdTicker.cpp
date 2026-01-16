#include "GsdTicker.h"

void GsdTicker::start(uint32_t ms) {
    _ticker.attach_ms(ms, tick, &_hasTicked);
}

void GsdTicker::stop() {
    _ticker.detach();
}

bool GsdTicker::ticked() {
    if (!_hasTicked)
        return false;
    _hasTicked = false;
    return true;
}

bool GsdTicker::active() {
    return _ticker.active();
}

void GsdTicker::tick(volatile bool* hasTicked) {
    *hasTicked = true;
}