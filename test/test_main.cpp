extern "C" {
#include <mavlink/v2.0/common/mavlink.h>
}
#include <GsdCore/MavlinkGateway.h>
#include <etl/optional.h>
#include <etl/string.h>
#include <etl/vector.h>
#include <unity.h>

using namespace gsd;

struct MockMavSocket : public IMavSocket {
    MavPacket readPacket;
    etl::vector<MavPacket, 10> writePackets;
    mavlink_message_t msg;
    etl::string<64> password = "admin";
    bool isConnected = false;

    void write(const MavPacket& packet, bool discreet) override { writePackets.push_back(packet); }

    bool read(MavPacket& packet) override {
        isConnected = true;
        packet = readPacket;
        return true;
    }

    bool peerAlive() { return true; };

    void changePassword(const char* oldPassword, const char* newPassword) override {
        if (oldPassword == password)
            password = newPassword;
    }

    void setHeartbeat() {
        readPacket.resize(MAVLINK_MAX_PACKET_LEN);
        mavlink_msg_heartbeat_pack_chan(1, 1, MAVLINK_COMM_0, &msg, 0, 0, 0, 0, 0);
        const uint16_t len = mavlink_msg_to_send_buffer(readPacket.data(), &msg);
        readPacket.resize(static_cast<size_t>(len));
    }

    void setDataRequest() {
        readPacket.resize(MAVLINK_MAX_PACKET_LEN);
        mavlink_msg_request_data_stream_pack(1, 1, &msg, 255, 1, MAV_DATA_STREAM_ALL, 30, 1);
        const uint16_t len = mavlink_msg_to_send_buffer(readPacket.data(), &msg);
        readPacket.resize(len);
    }

    void setManualControl(int16_t x, int16_t r) {
        readPacket.resize(MAVLINK_MAX_PACKET_LEN);
        mavlink_msg_manual_control_pack(1, 1, &msg, 0, x, 0, 0, r, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        const uint16_t len = mavlink_msg_to_send_buffer(readPacket.data(), &msg);
        readPacket.resize(len);
    }

    void setWifiConfig(const char* oldPwd, const char* newPwd) {
        readPacket.resize(MAVLINK_MAX_PACKET_LEN);
        mavlink_msg_wifi_config_ap_pack(1, 1, &msg, oldPwd, newPwd, 0, 0);
        const uint16_t len = mavlink_msg_to_send_buffer(readPacket.data(), &msg);
        readPacket.resize(len);
    }
};

struct MockSensors : ISensors {
    etl::optional<Gps> gps = Gps{1, 1, 1, 1, 1};
    int8_t batteryPercentage = 100;
    bool batteryOk = true;
    bool gpsOk = true;

    etl::optional<Gps> getGps() override { return gps; }

    int8_t getBatteryPercentage() override { return batteryPercentage; }

    bool isBatteryOk() override { return batteryOk; }

    bool isGpsOk() override { return gpsOk; }
};

struct MockDrive : IDrive {
    bool ok = true;
    int16_t forward, yaw = 0;

    void move(int16_t forward, int16_t yaw) override {
        this->forward = forward;
        this->yaw = yaw;
    }

    void recover() override {}

    bool isOk() override { return ok; }
};

struct MockVideoStream : IVideoStream {
    Url url = "url";
    bool isStreaming = false;
    bool ok = true;

    Url start() override {
        isStreaming = true;
        return url;
    }

    void stop() override { isStreaming = false; }

    bool isOk() override { return ok; }
};

struct MockSecurity : public ISecurity {
    Key getKey() { return {}; }
    uint64_t getTimestamp() { return 0; }
};

struct MockTicker : public ITicker {
    void start(uint32_t ms) override {}
    void stop() override {}
    bool ticked() override { return true; }
    bool active() override { return true; }
};

void parseMavPacket(const MavPacket& packet, mavlink_message_t& msg) {
    mavlink_status_t status;
    for (int i = 0; i < packet.size(); i++) {
        mavlink_parse_char(MAVLINK_COMM_0, packet[i], &msg, &status);
    }
}

MockMavSocket* socket;
MockSensors* sensors;
MockVideoStream* videoStream;
MockDrive* drive;
MockSecurity* security;
MavlinkGateway<MockTicker>* mavGateway;

void setUp() {
    socket = new MockMavSocket();
    sensors = new MockSensors();
    videoStream = new MockVideoStream();
    drive = new MockDrive();
    security = new MockSecurity();

    mavGateway =
        new MavlinkGateway<MockTicker>({}, *socket, *sensors, *videoStream, *drive, *security);
}

void tearDown() {
    delete mavGateway;
    delete drive;
    delete videoStream;
    delete sensors;
    delete socket;
    delete security;
}

void connectionEstablishedOnValidMessage() {
    socket->setHeartbeat();
    mavGateway->update();
    TEST_ASSERT(socket->isConnected);
}

void videoStreamStartedAndValidDataSentAfterRequest() {
    socket->writePackets.clear();
    socket->setDataRequest();
    mavGateway->update();

    TEST_ASSERT_EQUAL(4, socket->writePackets.size());

    mavlink_message_t msg;
    parseMavPacket(socket->writePackets[2], msg);
    mavlink_gps_raw_int_t gpsRaw;
    mavlink_msg_gps_raw_int_decode(&msg, &gpsRaw);
    parseMavPacket(socket->writePackets[3], msg);
    mavlink_battery_status_t batStatus;
    mavlink_msg_battery_status_decode(&msg, &batStatus);
    mavlink_video_stream_information_t videoStreamInfo;
    parseMavPacket(socket->writePackets[0], msg);
    mavlink_msg_video_stream_information_decode(&msg, &videoStreamInfo);

    TEST_ASSERT(videoStream->isStreaming);
    TEST_ASSERT_EQUAL_STRING(videoStream->url.c_str(), videoStreamInfo.uri);
    TEST_ASSERT_EQUAL(sensors->gps.value().altitude, gpsRaw.alt);
    TEST_ASSERT_EQUAL(sensors->gps.value().longitude, gpsRaw.lon);
    TEST_ASSERT_EQUAL(sensors->gps.value().latitude, gpsRaw.lat);
    TEST_ASSERT_EQUAL(sensors->gps.value().velocity, gpsRaw.vel);
    TEST_ASSERT_EQUAL(sensors->gps.value().cog, gpsRaw.cog);
    TEST_ASSERT_EQUAL(sensors->batteryPercentage, batStatus.battery_remaining);
}

void driveMovedAfterManualControlCommand() {
    int16_t forward = 10;
    int16_t yaw = 10;

    socket->setManualControl(forward, yaw);
    mavGateway->update();

    TEST_ASSERT_EQUAL(forward, drive->forward);
    TEST_ASSERT_EQUAL(yaw, drive->yaw);
}

void vehicleStateChangedAfterMessages() {
    mavlink_message_t msg;
    mavlink_heartbeat_t heartbeat;

    socket->writePackets.clear();
    socket->setHeartbeat();
    mavGateway->update();
    parseMavPacket(socket->writePackets[0], msg);
    mavlink_msg_heartbeat_decode(&msg, &heartbeat);
    TEST_ASSERT_EQUAL(MAV_STATE_ACTIVE, heartbeat.system_status);

    socket->writePackets.clear();
    socket->setManualControl(0, 0);
    drive->ok = false;
    sensors->batteryOk = true;
    sensors->gpsOk = true;
    videoStream->ok = true;
    mavGateway->update();
    parseMavPacket(socket->writePackets[0], msg);
    mavlink_msg_heartbeat_decode(&msg, &heartbeat);
    TEST_ASSERT_EQUAL(MAV_STATE_EMERGENCY, heartbeat.system_status);

    socket->writePackets.clear();
    socket->setDataRequest();
    drive->ok = true;
    sensors->batteryOk = true;
    sensors->gpsOk = false;
    videoStream->ok = true;
    mavGateway->update();
    parseMavPacket(socket->writePackets[1], msg);
    mavlink_msg_heartbeat_decode(&msg, &heartbeat);
    TEST_ASSERT_EQUAL(MAV_STATE_CRITICAL, heartbeat.system_status);

    socket->writePackets.clear();
    socket->setDataRequest();
    drive->ok = true;
    sensors->batteryOk = true;
    sensors->gpsOk = false;
    videoStream->ok = false;
    mavGateway->update();
    parseMavPacket(socket->writePackets[1], msg);
    mavlink_msg_heartbeat_decode(&msg, &heartbeat);
    TEST_ASSERT_EQUAL(MAV_STATE_EMERGENCY, heartbeat.system_status);

    socket->writePackets.clear();
    socket->setDataRequest();
    drive->ok = true;
    sensors->batteryOk = true;
    sensors->gpsOk = true;
    videoStream->ok = false;
    mavGateway->update();
    parseMavPacket(socket->writePackets[1], msg);
    mavlink_msg_heartbeat_decode(&msg, &heartbeat);
    TEST_ASSERT_EQUAL(MAV_STATE_CRITICAL, heartbeat.system_status);
}

void passwordChanged() {
    const char* newPassword = "nimda";
    socket->setWifiConfig(socket->password.c_str(), newPassword);
    mavGateway->update();
    TEST_ASSERT(socket->password == newPassword);
}

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(connectionEstablishedOnValidMessage);
    RUN_TEST(videoStreamStartedAndValidDataSentAfterRequest);
    RUN_TEST(driveMovedAfterManualControlCommand);
    RUN_TEST(vehicleStateChangedAfterMessages);
    RUN_TEST(passwordChanged);
    UNITY_END();
}