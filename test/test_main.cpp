extern "C" {
#include <mavlink/v2.0/common/mavlink.h>
}
#include <Gsd/MavlinkGateway.h>
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

    void write(const MavPacket& packet) override { writePackets.push_back(packet); }

    bool read(MavPacket& packet) override {
        packet = readPacket;
        return true;
    }

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
    etl::optional<Geo> geo = Geo{1, 1, 1, 1, 1};
    int8_t batteryPercentage = 100;
    bool ret = true;

    etl::optional<Geo> getGeo() override { return geo; }

    int8_t getBatteryPercentage() override { return batteryPercentage; }

    uint64_t getTimestamp() override { return 0; }

    bool isOk() override { return ret; }
};

struct MockDrive : IDrive {
    bool ret = true;
    int16_t forward, yaw = 0;

    void move(int16_t forward, int16_t yaw) {
        this->forward = forward;
        this->yaw = yaw;
    }

    bool isOk() override { return ret; }
};

struct MockVideoStream : IVideoStream {
    Url url = "url";
    bool ret = false;

    Url start() override {
        ret = true;
        return url;
    }

    void stop() override { ret = false; }

    bool isStreaming() override { return ret; }
};

struct MockSecurityKeyProvider : ISecurityKeyProvider {
    const Key& getKey() {}
};

struct MockNotifier : public INotifier {
    void notifyEvery(uint32_t ms, Condition& condition) override { condition.notify(); }
    void notifyOnce(uint32_t ms, Condition& condition) override {}
    void stop() override {}
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
MockSecurityKeyProvider* keyProvider;
MavlinkGateway<MockNotifier>* mavGateway;

void setUp() {
    socket = new MockMavSocket();
    sensors = new MockSensors();
    videoStream = new MockVideoStream();
    drive = new MockDrive();

    mavGateway = new MavlinkGateway<MockNotifier>({.connectionTimeoutMs = 200}, *socket, *sensors,
                                                  *videoStream, *drive, *keyProvider);
}

void tearDown() {
    delete mavGateway;
    delete drive;
    delete videoStream;
    delete sensors;
    delete socket;
}

void connectionEstablishedOnValidMessage() {
    socket->setHeartbeat();
    mavGateway->update();
    TEST_ASSERT(mavGateway->isConnected());
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

    TEST_ASSERT(videoStream->isStreaming());
    TEST_ASSERT_EQUAL(sensors->geo.value().altitude, gpsRaw.alt);
    TEST_ASSERT_EQUAL(sensors->geo.value().longitude, gpsRaw.lon);
    TEST_ASSERT_EQUAL(sensors->geo.value().latitude, gpsRaw.lat);
    TEST_ASSERT_EQUAL(sensors->geo.value().velocity, gpsRaw.vel);
    TEST_ASSERT_EQUAL(sensors->geo.value().cog, gpsRaw.cog);
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
    socket->setHeartbeat();
    mavGateway->update();
    TEST_ASSERT_EQUAL(MavlinkGateway<MockNotifier>::VehicleState::Ok, mavGateway->vehicleState());

    socket->setManualControl(0, 0);
    drive->ret = false;
    mavGateway->update();
    TEST_ASSERT_EQUAL(MavlinkGateway<MockNotifier>::VehicleState::Emergency,
                      mavGateway->vehicleState());

    socket->setDataRequest();
    drive->ret = true;
    mavGateway->update();
    TEST_ASSERT_EQUAL(MavlinkGateway<MockNotifier>::VehicleState::Critical,
                      mavGateway->vehicleState());

    socket->setDataRequest();
    videoStream->ret = false;
    mavGateway->update();
    TEST_ASSERT_EQUAL(MavlinkGateway<MockNotifier>::VehicleState::Critical,
                      mavGateway->vehicleState());
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