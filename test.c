#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "process_status.h"
#include "processes.h"

using ::testing::_;
using ::testing::Return;

class ProcessStatusMock : public ProcessStatus {
 public:
  MOCK_METHOD0(createCpuHistory, void());
  MOCK_METHOD2(updateCpuHistory, void(ProcessStatus*, int));
  MOCK_METHOD0(getPid, int());
  MOCK_METHOD1(getCpuhist, unsigned(int));
  MOCK_METHOD0(getHistidx, int());
  MOCK_METHOD0(getUtime, unsigned long long());
  MOCK_METHOD0(getStime, unsigned long long());
  MOCK_METHOD0(getState, char());
  MOCK_METHOD0(getNice, long());
  MOCK_METHOD0(getComm, char*());
  MOCK_METHOD0(getCpu, unsigned());
  MOCK_METHOD0(getRss, long());
};

TEST(process_status, nonExistingDirectoryShouldGiveEmptyProcessList) {
    processes target("qwerty");
    EXPECT_EQ(0, target.size());
}

TEST(process_status, single) {
    processes target("fixtures/single");
    EXPECT_EQ(1, target.size());
}

TEST(process_status, shouldCallCreateCpuHistoryIfOldListIsEmpty) {
    processes target("fixtures/empty");
    ProcessStatusMock mock;
    target.push_back(&mock);
    EXPECT_CALL(mock, createCpuHistory());
    /* EXPECT_CALL(mock, getPid()).WillOnce(Return(123)); */

    EXPECT_EQ(1, target.size());
    processes that("fixtures/empty");
    target.diff(that, 1);
}

TEST(process_status, shouldCallUpdateCpuHistoryIfOldListContainsProcess) {
    processes target("fixtures/empty");
    processes that("fixtures/empty");
    ProcessStatusMock mock1;
    target.push_back(&mock1);
    ProcessStatusMock mock2;
    that.push_back(&mock2);
    EXPECT_CALL(mock1, updateCpuHistory(_, _));
    EXPECT_CALL(mock1, getPid()).WillRepeatedly(Return(123));
    EXPECT_CALL(mock2, getPid()).WillRepeatedly(Return(123));

    EXPECT_EQ(1, target.size());
    target.diff(that, 1);
}

TEST(process_status, diffEmpty) {
    processes target("fixtures/empty");
    processes that("fixtures/empty");

    EXPECT_EQ(0, target.size());
    target.diff(that, 1);
    EXPECT_EQ(0, target.size());
}
