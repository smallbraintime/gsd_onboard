#include "GsdSystem.h"

GsdSystem::GsdSystem(const Config& config)
    : _config(config),
      _socket(config.network),
      _drive(_config.hardware.escLeftPin, _config.hardware.escRightPin),
      _sensors(config.hardware.gpsRxPin,
               config.hardware.batteryRxPin,
               config.hardware.batteryMaxMv,
               config.hardware.batteryMinMv,
               config.hardware.voltageDivider) {
    _videoStream = new VideoStream(config.hardware.camera);

    _mavGateway = new gsd::MavlinkGateway<GsdTicker>({.msgSigning = config.msgSigning}, _socket,
                                                     _sensors, *_videoStream, _drive, _security);
}

GsdSystem::~GsdSystem() {
    delete _mavGateway;
    delete _videoStream;
}

void GsdSystem::begin() {
    _drive.begin();
    _sensors.begin();
    _videoStream->begin();
    _socket.begin();
    _videoStream->start();
    xTaskCreatePinnedToCore(mavGatewayUpdate, "mavGateway", 8192, _mavGateway, 1, &_taskHandle, 1);
}

void GsdSystem::stop() {
    _socket.stop();
}

void GsdSystem::update() {
    // if (_mavGateway)
    //     _mavGateway->update();
    // vTaskDelay(pdMS_TO_TICKS(25));
}

void GsdSystem::mavGatewayUpdate(void* pvParameters) {
    gsd::MavlinkGateway<GsdTicker>* mavGateway =
        static_cast<gsd::MavlinkGateway<GsdTicker>*>(pvParameters);
    while (true) {
        if (mavGateway)
            mavGateway->update();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}