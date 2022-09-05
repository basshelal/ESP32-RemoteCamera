#ifndef ESP32_REMOTECAMERA_TEST_TESTUTILS_H
#define ESP32_REMOTECAMERA_TEST_TESTUTILS_H

#include "unity.h"

#define MESSAGE(format, ...) \
({char buffer[128];          \
sprintf(buffer, format, ##__VA_ARGS__); \
buffer;})

#define XTEST_CASE(name, desc) \
__attribute__((unused))        \
static void UNITY_TEST_UID(disabled_test_func_) (void)

#define ASSERT(condition, message, ...) \
do{UNITY_TEST_ASSERT(condition, __LINE__, MESSAGE(message, ##__VA_ARGS__));}while(0)

#define ASSERT_FALSE(condition, message, ...) \
do{UNITY_TEST_ASSERT(!(condition), __LINE__, MESSAGE(message, ##__VA_ARGS__));}while(0)

#define ASSERT_NOT_NULL(pointer, message, ...) \
do{UNITY_TEST_ASSERT_NOT_NULL(pointer, __LINE__, MESSAGE(message, ##__VA_ARGS__));}while(0)

#define ASSERT_NULL(pointer, message, ...) \
do{UNITY_TEST_ASSERT_NULL(pointer, __LINE__, MESSAGE(message, ##__VA_ARGS__));}while(0)

#define ASSERT_INT_EQUAL(expected, actual, message, ...) \
do{UNITY_TEST_ASSERT_EQUAL_INT(expected, actual, __LINE__, MESSAGE(message, ##__VA_ARGS__));}while(0)

#define ASSERT_UINT_EQUAL(expected, actual, message, ...) \
do{UNITY_TEST_ASSERT_EQUAL_UINT(expected, actual, __LINE__, MESSAGE(message, ##__VA_ARGS__));}while(0)

#define ASSERT_STRING_EQUAL(expected, actual, message, ...) \
do{UNITY_TEST_ASSERT_EQUAL_STRING(expected, actual, __LINE__, MESSAGE(message, ##__VA_ARGS__));}while(0)

#define beginTestFile(fileName) \
__attribute__((weak)) void beforeEach##__FILENAME__();\
__attribute__((weak)) void afterEach##__FILENAME__();

#define beforeEach() \
void beforeEach##__FILENAME__()

#define afterEach() \
void afterEach##__FILENAME__()

#define beginTest() \
do {if(beforeEach##__FILENAME__){\
beforeEach##__FILENAME__(); \
}} while(0)

#define endTest() \
do {if(afterEach##__FILENAME__){\
afterEach##__FILENAME__(); \
}} while(0)

#define endTestFile

#define testCase(name) TEST_CASE(name, "")
#define xtestCase(name) XTEST_CASE(name, "")

#define ignoreTest() TEST_IGNORE()

#endif //ESP32_REMOTECAMERA_TEST_TESTUTILS_H
