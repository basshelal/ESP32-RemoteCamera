#include "unity.h"
#include "TestUtils.h"
#include "LogList.h"

#define TEST_TAG "[LogList]"
#define TEST(name) TEST_CASE(name, TEST_TAG)
#define XTEST(name) XTEST_CASE(name, TEST_TAG)

static LogList *logList_createDefault() {
    LogListOptions options = LOG_LIST_DEFAULT_OPTIONS;
    LogList *logList = logList_create(&options);
    return logList;
}

TEST("LogList create") {
    LogListOptions options = LOG_LIST_DEFAULT_OPTIONS;
    LogList *logList = logList_create(&options);
    ASSERT_NOT_NULL(logList, "LogList should not be NULL");
    logList_destroy(logList);
}

TEST("LogList append") {
    LogList *logList = logList_createDefault();
    logList_append(logList, "0");
    logList_append(logList, "1");
    logList_append(logList, "2");

    capacity_t size = logList_getSize(logList);
    ASSERT_UINT_EQUAL(3, size, "size was incorrect");
    logList_destroy(logList);
}

TEST("LogList getSize") {
    capacity_t size = logList_getSize(NULL);
    ASSERT_UINT_EQUAL(LIST_INVALID_INDEX_CAPACITY, size, "size was incorrect");

    LogList *logList = logList_createDefault();
    size = logList_getSize(logList);
    ASSERT_UINT_EQUAL(0, size, "size was incorrect");

    logList_append(logList, "0");
    size = logList_getSize(logList);
    ASSERT_UINT_EQUAL(1, size, "size was incorrect");
    logList_destroy(logList);
}

TEST("LogList getCapacity") {
    capacity_t capacity = logList_getCapacity(NULL);
    ASSERT_UINT_EQUAL(LIST_INVALID_INDEX_CAPACITY, capacity, "capacity was incorrect");

    LogList *logList = logList_createDefault();
    capacity = logList_getCapacity(logList);
    ASSERT_UINT_EQUAL(LOG_LIST_DEFAULT_CAPACITY, capacity, "capacity was incorrect");

    logList_destroy(logList);
}

TEST("LogList clear") {
    LogList *logList = logList_createDefault();
    logList_append(logList, "0");
    logList_append(logList, "1");
    logList_append(logList, "2");

    capacity_t size = logList_getSize(logList);
    ASSERT_UINT_EQUAL(3, size, "size was incorrect");

    logList_clear(logList);
    size = logList_getSize(logList);
    ASSERT_UINT_EQUAL(0, size, "size was incorrect");

    logList_destroy(logList);
}

TEST("LogList getList") {
    LogList *logList = logList_createDefault();
    List *list = list_create();

    logList_append(logList, "0");
    logList_append(logList, "1");
    logList_append(logList, "2");
    logList_append(logList, "3");

    logList_getList(logList, list);

    ASSERT_UINT_EQUAL(4, list_getSize(list), "list size was incorrect");
    ASSERT_STRING_EQUAL("0", (const char *) list_getItem(list, 0), "list items were incorrect");

    logList_destroy(logList);
    list_destroy(list);
}

TEST("LogList exceed capacity") {
    LogListOptions options = LOG_LIST_DEFAULT_OPTIONS;
    options.capacity = 2;
    LogList *logList = logList_create(&options);
    List *list = list_create();

    logList_append(logList, "0");
    logList_append(logList, "1");
    logList_append(logList, "2");

    logList_getList(logList, list);
    ASSERT_UINT_EQUAL(2, list_getSize(list), "list size was incorrect");
    ASSERT_STRING_EQUAL("1", (const char *) list_getItem(list, 0), "list items were incorrect");
    ASSERT_STRING_EQUAL("2", (const char *) list_getItem(list, 1), "list items were incorrect");

    logList_destroy(logList);
}