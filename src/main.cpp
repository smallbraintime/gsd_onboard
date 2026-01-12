#include <Arduino.h>
#include <Gsd/Debug.h>
#include "DShotESC.h"

DShotESC esc0;
DShotESC esc1;

void setup() {
    GSD_DEBUG_INIT();

    esc0.install(GPIO_NUM_20, RMT_CHANNEL_0);
    esc1.install(GPIO_NUM_21, RMT_CHANNEL_1);
    esc0.init();
    esc1.init();
    esc0.setReversed(true);
    esc1.setReversed(true);
    esc0.set3DMode(true);
    esc1.set3DMode(true);

    for (int i = 0; i < 200; i++) {
        esc0.sendThrottle(0);
        esc1.sendThrottle(0);
        delay(10);
    }
}

void loop() {
    esc0.sendThrottle(100);
    esc1.sendThrottle(100);
    delay(1);
}