#ifndef ESP32_REMOTECAMERA_TEST_TESTUTILS_H
#define ESP32_REMOTECAMERA_TEST_TESTUTILS_H

#include "unity.h"

#define ASSERT_NOT_NULL(pointer, message) UNITY_TEST_ASSERT_NOT_NULL(pointer, __LINE__, message)

#define ASSERT_INT_EQUAL(expected, actual, message) UNITY_TEST_ASSERT_EQUAL_INT(expected, actual, __LINE__, message)

#define ASSERT_INT_GREATER_THAN(threshold, actual, message) UNITY_TEST_ASSERT_GREATER_THAN_INT(threshold, actual, __LINE__, message)

#define ASSERT_MEMORY_EQUAL(expected, actual, len, message) UNITY_TEST_ASSERT_EQUAL_MEMORY(expected, actual, len, __LINE__, message)

#endif //ESP32_REMOTECAMERA_TEST_TESTUTILS_H
