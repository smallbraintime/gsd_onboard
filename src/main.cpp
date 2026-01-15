#include <Arduino.h>

#include "Debug.h"
#include "GsdSystem.h"

GsdSystem sys({});

void setup() {
    GSD_INIT();
    sys.begin();
}

void loop() {
    sys.update();
}