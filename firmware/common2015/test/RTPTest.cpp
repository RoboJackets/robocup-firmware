#include <gtest/gtest.h>

#include "RTP.hpp"

#include <string>

TEST(RTP, Header_IsdefaultConstructable) { RTP::Header h; }
TEST(RTP, Packet_IsdefaultConstructable) { RTP::Packet p; }
TEST(RTP, ControlMessage_IsdefaultConstructable) { RTP::ControlMessage m; }
TEST(RTP, RobotStatusMessage_IsdefaultConstructable) {
    RTP::RobotStatusMessage m;
}

TEST(RTP, Header_IsCorrectSize) {
    EXPECT_EQ(sizeof(RTP::Header), 2);
    EXPECT_EQ(sizeof(RTP::Header), RTP::HeaderSize);
}

TEST(RTP, ControlMessage_IsCorrectSize) {
    EXPECT_EQ(sizeof(RTP::ControlMessage), 10);
}
TEST(RTP, RobotStatusMessage_IsCorrectSize) {
    EXPECT_EQ(sizeof(RTP::RobotStatusMessage), 3);
}

TEST(RTP, PortType_HasCorrectValues) {
    EXPECT_EQ(RTP::PortType::SINK, 0);
    EXPECT_EQ(RTP::PortType::LINK, 1);
    EXPECT_EQ(RTP::PortType::CONTROL, 2);
    EXPECT_EQ(RTP::PortType::LEGACY, 3);
    EXPECT_EQ(RTP::PortType::PING, 4);
}

TEST(RTP, MessageType_HasCorrectValues) {
    EXPECT_EQ(RTP::MessageType::CONTROL, 0);
    EXPECT_EQ(RTP::MessageType::TUNING, 1);
    EXPECT_EQ(RTP::MessageType::UPGRADE, 2);
    EXPECT_EQ(RTP::MessageType::MISC, 3);
}

TEST(RTP, Packet_HasCorrectSizesAfterConstruction) {
    RTP::Packet p1;
    EXPECT_EQ(p1.size(), RTP::HeaderSize);

    RTP::Packet p2{std::vector<uint8_t>{
        99, static_cast<uint8_t>(RTP::PortType::LEGACY), 1, 1, 1}};
    EXPECT_EQ(p2.size(), 5);
    EXPECT_EQ(p2.header.address, 99);
    EXPECT_EQ(p2.header.port, RTP::PortType::LEGACY);
    EXPECT_EQ(p2.header.type, RTP::MessageType::CONTROL);

    RTP::Packet p3{{1, 2, 3}};
    EXPECT_EQ(p3.size(), RTP::HeaderSize + 3);
    EXPECT_EQ(p3.header.address, RTP::INVALID_ROBOT_UID);
    EXPECT_EQ(p3.header.port, RTP::PortType::PING);
    EXPECT_EQ(p3.header.type, RTP::MessageType::MISC);

    const std::string msg = "Hello World!";
    RTP::Packet p4(msg);
    EXPECT_EQ(p4.size(), RTP::HeaderSize + msg.size() + 1);
    EXPECT_EQ(p4.header.address, RTP::INVALID_ROBOT_UID);
    EXPECT_EQ(p4.header.port, RTP::PortType::SINK);
    EXPECT_EQ(p4.header.type, RTP::MessageType::MISC);
}
