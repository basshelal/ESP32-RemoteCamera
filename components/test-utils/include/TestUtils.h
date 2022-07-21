#ifndef ESP32_REMOTECAMERA_TEST_TESTUTILS_H
#define ESP32_REMOTECAMERA_TEST_TESTUTILS_H

#include "unity.h"

#define MESSAGE(format, ...) \
({char buffer[128];          \
sprintf(buffer, format, ##__VA_ARGS__); \
buffer;})

#define BEFORE_EACH() void setUp(void)
#define AFTER_EACH() void tearDown(void)

#define ASSERT(condition, message, ...) UNITY_TEST_ASSERT(condition, __LINE__, MESSAGE(message, ##__VA_ARGS__))
#define ASSERT_NOT_NULL(pointer, message, ...) UNITY_TEST_ASSERT_NOT_NULL(pointer, __LINE__, MESSAGE(message, ##__VA_ARGS__))
#define ASSERT_INT_EQUAL(expected, actual, message, ...) UNITY_TEST_ASSERT_EQUAL_INT(expected, actual, __LINE__, MESSAGE(message, ##__VA_ARGS__))
#define ASSERT_INT_GREATER_THAN(threshold, actual, message, ...) UNITY_TEST_ASSERT_GREATER_THAN_INT(threshold, actual, __LINE__, MESSAGE(message, ##__VA_ARGS__))
#define ASSERT_STRING_EQUAL(expected, actual, message, ...) UNITY_TEST_ASSERT_EQUAL_STRING(expected, actual, __LINE__, MESSAGE(message, ##__VA_ARGS__))
#define ASSERT_MEMORY_EQUAL(expected, actual, len, message, ...) UNITY_TEST_ASSERT_EQUAL_MEMORY(expected, actual, len, __LINE__, MESSAGE(message, ##__VA_ARGS__))

#endif //ESP32_REMOTECAMERA_TEST_TESTUTILS_H
