#include "Notifier.h"

void Notifier::notifyEvery(uint32_t ms, gsd::Condition* condition) {
    _ticker.attach_ms(ms, notify, condition);
}

void Notifier::notifyOnce(uint32_t ms, gsd::Condition* condition) {
    _ticker.once_ms(ms, notify, condition);
}

void Notifier::stop() {
    _ticker.detach();
}

void Notifier::notify(gsd::Condition* condition) {
    condition->notify();
}