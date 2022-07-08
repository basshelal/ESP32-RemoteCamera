#include "../include/List.h"
#include "unity.h"

TEST_CASE("Test List Creation", "[list]") {
    List *list = list_create();
    UNITY_TEST_ASSERT_NOT_NULL(list, "This is the line", "List should be non-null");
    list_destroy(list);
    list = NULL;
    UNITY_TEST_ASSERT_NOT_NULL(list, "Line again", "List should be non-null again");
}