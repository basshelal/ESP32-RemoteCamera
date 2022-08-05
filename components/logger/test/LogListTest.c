#include "unity.h"
#include "TestUtils.h"
#include "LogList.h"

#define TEST_TAG "[LogList]"
#define TEST(name) TEST_CASE(name, TEST_TAG)
#define XTEST(name) XTEST_CASE(name, TEST_TAG)

TEST("LogList create") {
    LogListOptions options = LOG_LIST_DEFAULT_OPTIONS;
    LogList *logList = logList_create(&options);
    ASSERT_NOT_NULL(logList, "LogList should not be NULL");
    logList_destroy(logList);
}