#ifndef ESP32_REMOTECAMERA_TEST_ASSERTIONS_H
#define ESP32_REMOTECAMERA_TEST_ASSERTIONS_H

#include "unity.h"

#define assertTrue(condition) do{TEST_ASSERT_TRUE(condition);}while(0)

#define assertFalse(condition) do{TEST_ASSERT_FALSE(condition);}while(0)

#define assertNotNull(pointer) do{TEST_ASSERT_NOT_NULL(pointer);}while(0)

#define assertNull(pointer) do{TEST_ASSERT_NULL(pointer);}while(0)

#define assertEqualInt(expected, actual) \
do{TEST_ASSERT_EQUAL_INT(expected, actual);}while(0)

#define assertNotEqualInt(expected, actual) \
do{TEST_ASSERT_NOT_EQUAL(expected, actual);}while(0)

#define assertOK(func) assertEqualInt(0,func)

#define assertError(func) assertNotEqualInt(0,func)

#define assertEqualUInt64(expected, actual) \
do{TEST_ASSERT_EQUAL_UINT64(expected, actual);}while(0)

#define assertEqualUInt(expected, actual) \
do{TEST_ASSERT_EQUAL_UINT(expected, actual);}while(0)

#define assertGreaterThanInt(threshold, actual, message, ...) \
do{UNITY_TEST_ASSERT_GREATER_THAN_INT(threshold, actual, __LINE__, MESSAGE(message, ##__VA_ARGS__));}while(0)

#define assertEqualString(expected, actual, message, ...) \
do{UNITY_TEST_ASSERT_EQUAL_STRING(expected, actual, __LINE__, MESSAGE(message, ##__VA_ARGS__));}while(0)

#define assertEqualPointer(expected, actual, len, message, ...) \
do{UNITY_TEST_ASSERT_EQUAL_MEMORY(expected, actual, len, __LINE__, MESSAGE(message, ##__VA_ARGS__));}while(0)

#endif //ESP32_REMOTECAMERA_TEST_ASSERTIONS_H
