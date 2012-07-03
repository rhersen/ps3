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

    EXPECT_EQ(1, target.size());
    processes that("fixtures/empty");
    target.diff(that, 1);
}

TEST(process_status, shouldCallUpdateCpuHistoryIfOldListContainsProcess) {
    processes target("fixtures/empty");
    processes that("fixtures/empty");
    ProcessStatusMock newProcess;
    target.push_back(&newProcess);
    ProcessStatusMock oldProcess;
    that.push_back(&oldProcess);
    EXPECT_CALL(oldProcess, getPid()).WillRepeatedly(Return(2));
    EXPECT_CALL(newProcess, updateCpuHistory(_, _));
    EXPECT_CALL(newProcess, getPid()).WillRepeatedly(Return(2));

    EXPECT_EQ(1, target.size());
    target.diff(that, 1);
}

TEST(process_status, shouldCallUpdateCpuHistoryIfOldListDoesNotContainProcess) {
    processes target("fixtures/empty");
    processes that("fixtures/empty");
    ProcessStatusMock newProcess;
    target.push_back(&newProcess);

    ProcessStatusMock old1;
    that.push_back(&old1);
    EXPECT_CALL(old1, getPid()).WillRepeatedly(Return(2));

    ProcessStatusMock old2;
    that.push_back(&old2);
    EXPECT_CALL(old2, getPid()).WillRepeatedly(Return(4));

    EXPECT_CALL(newProcess, createCpuHistory());
    EXPECT_CALL(newProcess, getPid()).WillRepeatedly(Return(3));

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
