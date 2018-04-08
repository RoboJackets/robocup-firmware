#include <gtest/gtest.h>

#include "rc-fshare/rtp.hpp"

#include <string>

TEST(rtp, Header_IsdefaultConstructable) { rtp::Header h; }
TEST(rtp, Packet_IsdefaultConstructable) { rtp::Packet p; }
TEST(rtp, ControlMessage_IsdefaultConstructable) { rtp::ControlMessage m; }
TEST(rtp, RobotStatusMessage_IsdefaultConstructable) {
    rtp::RobotStatusMessage m;
}

TEST(rtp, Header_IsCorrectSize) {
    EXPECT_EQ(sizeof(rtp::Header), 2);
    EXPECT_EQ(sizeof(rtp::Header), rtp::HeaderSize);
}

// TODO: make sure this test is correct
TEST(rtp, ControlMessage_IsCorrectSize) {
    EXPECT_EQ(sizeof(rtp::ControlMessage), 9);
}
TEST(rtp, RobotStatusMessage_IsCorrectSize) {
    EXPECT_EQ(sizeof(rtp::RobotStatusMessage), 4);
}

TEST(rtp, PortType_HasCorrectValues) {
    EXPECT_EQ(rtp::PortType::SINK, 0);
    EXPECT_EQ(rtp::PortType::LINK, 1);
    EXPECT_EQ(rtp::PortType::CONTROL, 2);
    EXPECT_EQ(rtp::PortType::LEGACY, 3);
    EXPECT_EQ(rtp::PortType::PING, 4);
}

TEST(rtp, MessageType_HasCorrectValues) {
    EXPECT_EQ(rtp::MessageType::CONTROL, 0);
    EXPECT_EQ(rtp::MessageType::TUNING, 1);
    EXPECT_EQ(rtp::MessageType::UPGRADE, 2);
    EXPECT_EQ(rtp::MessageType::MISC, 3);
}

TEST(rtp, Packet_HasCorrectSizesAfterConstruction) {
    rtp::Packet p1;
    EXPECT_EQ(p1.size(), rtp::HeaderSize);

    rtp::Packet p2{std::vector<uint8_t>{
        99, static_cast<uint8_t>(rtp::PortType::LEGACY), 1, 1, 1}};
    EXPECT_EQ(p2.size(), 5);
    EXPECT_EQ(p2.header.address, 99);
    EXPECT_EQ(p2.header.port, rtp::PortType::LEGACY);
    EXPECT_EQ(p2.header.type, rtp::MessageType::CONTROL);

    rtp::Packet p3{{1, 2, 3}};
    EXPECT_EQ(p3.size(), rtp::HeaderSize + 3);
    EXPECT_EQ(p3.header.address, rtp::INVALID_ROBOT_UID);
    EXPECT_EQ(p3.header.port, rtp::PortType::PING);
    EXPECT_EQ(p3.header.type, rtp::MessageType::MISC);

    const std::string msg = "Hello World!";
    rtp::Packet p4(msg);
    EXPECT_EQ(p4.size(), rtp::HeaderSize + msg.size() + 1);
    EXPECT_EQ(p4.header.address, rtp::INVALID_ROBOT_UID);
    EXPECT_EQ(p4.header.port, rtp::PortType::SINK);
    EXPECT_EQ(p4.header.type, rtp::MessageType::MISC);
}
