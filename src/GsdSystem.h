#pragma once

#include <WiFi.h>
#include <esp_camera.h>

#include <GsdCore/MavlinkGateway.h>

#include "Drive.h"
#include "GsdTicker.h"
#include "MavSocket.h"
#include "Security.h"
#include "Sensors.h"
#include "VideoStream.h"

class GsdSystem {
   public:
    struct Config {
        MavSocket::Config network;
        struct {
            int32_t batteryMaxMv = 8400;
            int32_t batteryMinMv = 6400;
            float voltageDivider = 3.0f;

            uint8_t batteryRxPin = 14;
            uint8_t gpsRxPin = 0;

            uint8_t escLeftPin = 21;
            uint8_t escRightPin = 47;

            // camera
            uint8_t d7 = 16;
            uint8_t d6 = 17;
            uint8_t d5Pin = 18;
            uint8_t d4Pin = 12;
            uint8_t d3Pin = 10;
            uint8_t d2Pin = 8;
            uint8_t d1Pin = 9;
            uint8_t d0Pin = 11;
            uint8_t xclkPin = 15;
            uint8_t pclkPin = 13;
            uint8_t vsyncPin = 6;
            uint8_t hrefPin = 7;
            uint8_t sccbSdaPin = 4;
            uint8_t sccSclPin = 5;
        } hardware;
        bool msgSigning = false;
    };

    GsdSystem(const GsdSystem&) = delete;
    GsdSystem& operator=(const GsdSystem&) = delete;

    explicit GsdSystem(const Config& config)
        : _config(config),
          _socket(config.network),
          _sensors(config.hardware.batteryMaxMv,
                   config.hardware.batteryMinMv,
                   config.hardware.voltageDivider) {
        // camera_config_t camConfig{
        //     .pin_pwdn = -1,
        //     .pin_reset = -1,
        //     .pin_xclk = XCLK,
        //     .pin_sccb_sda = SCCB_SDA,
        //     .pin_sccb_scl = SCCB_SCL,
        //     .pin_d7 = D7,
        //     .pin_d6 = D6,
        //     .pin_d5 = D5,
        //     .pin_d4 = D4,
        //     .pin_d3 = D3,
        //     .pin_d2 = D2,
        //     .pin_d1 = D1,
        //     .pin_d0 = D0,
        //     .pin_vsync = VSYNC,
        //     .pin_href = HREF,
        //     .pin_pclk = PCLK,
        //     .xclk_freq_hz = 16000000,
        //     .ledc_timer = LEDC_TIMER_0,
        //     .ledc_channel = LEDC_CHANNEL_0,
        //     .pixel_format = PIXFORMAT_JPEG,
        //     .frame_size = FRAMESIZE_VGA,
        //     .jpeg_quality = 10,
        //     .fb_count = 2,
        //     .fb_location = psramFound() ? CAMERA_FB_IN_PSRAM : CAMERA_FB_IN_DRAM,
        //     .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
        // };
        camera_config_t camConfig;
        camConfig.ledc_channel = LEDC_CHANNEL_0;
        camConfig.ledc_timer = LEDC_TIMER_0;
        camConfig.pin_d0 = 11;
        camConfig.pin_d1 = 9;
        camConfig.pin_d2 = 8;
        camConfig.pin_d3 = 10;
        camConfig.pin_d4 = 12;
        camConfig.pin_d5 = 18;
        camConfig.pin_d6 = 17;
        camConfig.pin_d7 = 16;
        camConfig.pin_xclk = 15;
        camConfig.pin_pclk = 13;
        camConfig.pin_vsync = 6;
        camConfig.pin_href = 7;
        camConfig.pin_sccb_sda = 4;
        camConfig.pin_sccb_scl = 5;
        camConfig.pin_pwdn = -1;
        camConfig.pin_reset = -1;
        camConfig.xclk_freq_hz = 16000000;
        camConfig.pixel_format = PIXFORMAT_JPEG;
        camConfig.frame_size = FRAMESIZE_QVGA;
        camConfig.jpeg_quality = 10;
        camConfig.fb_count = 1;
        camConfig.fb_location = psramFound() ? CAMERA_FB_IN_PSRAM : CAMERA_FB_IN_DRAM,

        _videoStream = new VideoStream(camConfig);

        _mavGateway = new gsd::MavlinkGateway<GsdTicker>(
            {.msgSigning = config.msgSigning}, _socket, _sensors, *_videoStream, _drive, _security);
    }
    ~GsdSystem() {
        delete _mavGateway;
        delete _videoStream;
    }

    void begin() {
        _drive.begin(_config.hardware.escLeftPin, _config.hardware.escRightPin);
        _sensors.begin(_config.hardware.gpsRxPin, _config.hardware.batteryRxPin);
        _socket.begin();
    }
    void stop() { _socket.stop(); }
    void update() { _mavGateway->update(); }

   private:
    const Config _config;
    MavSocket _socket;
    Sensors _sensors;
    Drive _drive;
    Security _security;
    VideoStream* _videoStream;
    gsd::MavlinkGateway<GsdTicker>* _mavGateway;
};
