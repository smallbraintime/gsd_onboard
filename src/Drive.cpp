#include "Drive.h"

Drive::Drive(int8_t leftEscTxPin, int8_t rightEscTxPin)
    : _leftEscPin(leftEscTxPin), _rightEscPin(rightEscTxPin) {}

void Drive::begin() {
    _running = true;
    xTaskCreate(motorHandler, "motors", 8192, this, 10, &_taskHandle);
}

void Drive::end() {
    _running = false;
    vTaskDelete(_taskHandle);
}

void Drive::move(int16_t forward, int16_t yaw) {
    _forward = constrain(forward, 0, 1000);
    _yaw = constrain(yaw, -1000, 1000);
}

bool Drive::isOk() {
    return _isOk;
};

void Drive::motorHandler(void* pvParameters) {
    Drive* drive = static_cast<Drive*>(pvParameters);
    DShotESC leftEsc;
    DShotESC rightEsc;

    esp_err_t result = ESP_FAIL;

    vTaskDelay(pdMS_TO_TICKS(1000));
    result = leftEsc.install(static_cast<gpio_num_t>(drive->_leftEscPin), RMT_CHANNEL_0);
    result = rightEsc.install(static_cast<gpio_num_t>(drive->_rightEscPin), RMT_CHANNEL_1);
    if (result != 0)
        GSD_DEBUG("failed to install esc");

    result = leftEsc.init();
    result = rightEsc.init();
    if (result != 0)
        GSD_DEBUG("failed to init esc");

    for (int i = 0; i < 500; i++) {
        leftEsc.sendThrottle(0);
        rightEsc.sendThrottle(0);
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    while (drive->_running) {
        esp_err_t result = ESP_FAIL;

        int16_t f = drive->_forward.load();
        int16_t y = drive->_yaw.load();

        int16_t left = constrain(f - y, 0, 1000);
        int16_t right = constrain(f + y, 0, 1000);

        leftEsc.sendThrottle(left);
        rightEsc.sendThrottle(right);

        if (result != 0)
            drive->_isOk = false;
        else
            drive->_isOk = true;

        vTaskDelay(pdMS_TO_TICKS(1));
    }

    leftEsc.uninstall();
    rightEsc.uninstall();

    drive->_isOk = false;
}