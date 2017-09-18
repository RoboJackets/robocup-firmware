#include <gtest/gtest.h>

#include "CommPort.hpp"
#include "firmware-common/common2015/utils/rtp.hpp"

#include <functional>

namespace {
using CommPortT = CommPort<void(), void()>;

void testRxCallback() { return; }
void testTxCallback() { return; }
}

TEST(CommPort, IsdefaultConstructable) { CommPortT p; }

TEST(CommPort, IsBindableAndUnbindable) {
    CommPortT p;

    EXPECT_FALSE(p.hasRxCallback());
    EXPECT_FALSE(p.hasTxCallback());

    p.setRxCallback(testRxCallback);
    EXPECT_TRUE(p.hasRxCallback());
    p.setRxCallback(nullptr);
    EXPECT_FALSE(p.hasRxCallback());

    p.setTxCallback(testTxCallback);
    EXPECT_TRUE(p.hasTxCallback());
    p.setTxCallback(nullptr);
    EXPECT_FALSE(p.hasTxCallback());
}

TEST(CommPort, HandlesCounters) {
    CommPortT p;

    EXPECT_EQ(p.getRxCount(), 0);
    EXPECT_EQ(p.getTxCount(), 0);

    p.getRxCallback();
    EXPECT_EQ(p.getRxCount(), 1);
    p.getTxCallback();
    EXPECT_EQ(p.getTxCount(), 1);

    p.resetCounts();
    EXPECT_EQ(p.getRxCount(), 0);
    EXPECT_EQ(p.getTxCount(), 0);
}
