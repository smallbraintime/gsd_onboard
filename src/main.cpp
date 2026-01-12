#include <Arduino.h>

#include "GsdSystem.h"

GsdSystem sys({});

void setup() {
    Serial.begin(SERIAL_BAUD);
}

void loop() {
    sys.update();
}