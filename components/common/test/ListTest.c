#include <memory.h>
#include "List.h"
#include "unity.h"

// TODO: 09-Jul-2022 @basshelal: Cleaner smaller tests, use macro helpers if needed

#define LIST_TAG "[list]"

int myInt = 69;

TEST_CASE("Create List", LIST_TAG) {
    List *list = list_create();
    UNITY_TEST_ASSERT_NOT_NULL(list, __LINE__, "List should be non-null");
    list_destroy(list);
}

TEST_CASE("Create List with Options Capacity and isGrowable", LIST_TAG) {
    ListOptions listOptions = {
            .capacity = 1,
            .isGrowable = false,
            .growthFactor = 1,
            .errorCallback = NULL
    };
    List *list = list_createOptions(&listOptions);
    UNITY_TEST_ASSERT_NOT_NULL(list, __LINE__, "List should be non-null");

    list_addItem(list, &myInt);
    list_addItem(list, &myInt);
    list_addItem(list, &myInt);

    UNITY_TEST_ASSERT_EQUAL_INT(listOptions.capacity, list_size(list), __LINE__,
                                "List size should not be larger than it's capacity");
    list_destroy(list);
}

TEST_CASE("Create List with Options growth factor", LIST_TAG) {
    ListOptions listOptions = {
            .capacity = 1,
            .isGrowable = true,
            .growthFactor = 2,
            .errorCallback = NULL
    };
    List *list = list_createOptions(&listOptions);
    UNITY_TEST_ASSERT_NOT_NULL(list, __LINE__, "List should be non-null");

    list_addItem(list, &myInt);
    list_addItem(list, &myInt);

    UNITY_TEST_ASSERT_EQUAL_INT(2, list_size(list), __LINE__,
                                "List size should be 2");
    list_destroy(list);
}

int callbackCalled = 0;

void error_callback(const ListError listError) {
    callbackCalled++;
}

TEST_CASE("Create List with Options error callback", LIST_TAG) {
    ListOptions listOptions = {
            .capacity = 1,
            .isGrowable = false,
            .growthFactor = 2,
            .errorCallback = error_callback
    };
    List *list = list_createOptions(&listOptions);
    UNITY_TEST_ASSERT_NOT_NULL(list, __LINE__, "List should be non-null");

    list_addItem(list, &myInt);
    list_addItem(list, &myInt);

    UNITY_TEST_ASSERT_GREATER_THAN_INT(0, callbackCalled, __LINE__, "Error callback should have been called");
    list_destroy(list);
    callbackCalled = 0;
}

TEST_CASE("List Size", LIST_TAG) {
    UNITY_TEST_ASSERT_EQUAL_INT(-1, list_size(NULL), __LINE__, "Null list size should be -1");

    List *list = list_create();

    const int mySize = 100;
    for (int i = 0; i < mySize; i++) {
        list_addItem(list, &myInt);
    }

    UNITY_TEST_ASSERT_EQUAL_INT(mySize, list_size(list), __LINE__, "List size was incorrect");

    list_destroy(list);
}

TEST_CASE("List isEmpty", LIST_TAG) {
    UNITY_TEST_ASSERT_EQUAL_INT(true, list_isEmpty(NULL), __LINE__, "Null List isEmpty should be true");

    List *list = list_create();

    UNITY_TEST_ASSERT_EQUAL_INT(true, list_isEmpty(list), __LINE__, "List isEmpty should be true");

    list_addItem(list, &myInt);

    UNITY_TEST_ASSERT_EQUAL_INT(false, list_isEmpty(list), __LINE__, "List isEmpty should be false");

    list_destroy(list);
}

TEST_CASE("List get item", LIST_TAG) {
    List *list = list_create();

    list_addItem(list, &myInt);

    int *result = (int *) list_getItem(list, 0);

    UNITY_TEST_ASSERT_NOT_NULL(result, __LINE__, "Result should not be null");

    UNITY_TEST_ASSERT_EQUAL_INT(myInt, *result, __LINE__, "List get item was incorrect");

    list_destroy(list);
}

TEST_CASE("Access items after list destroy", LIST_TAG) {
    List *list = list_create();

    const int itemCount = 128;
    const int size = (int) (sizeof(int) * itemCount);
    int *myItem = calloc(1, size);
    for (int i = 0; i < itemCount; i++) {
        myItem[i * sizeof(int)] = myInt;
    }

    list_addItem(list, myItem);

    UNITY_TEST_ASSERT_EQUAL_MEMORY(myItem, list_getItem(list, 0), size, __LINE__, "Items should be identical");

    list_destroy(list);

    list = NULL;

    UNITY_TEST_ASSERT_NOT_NULL(myItem, __LINE__, "Pointer should not be null");

    char stringBuffer[128];
    for (int i = 0; i < itemCount; i++) {
        snprintf(stringBuffer, 128, "Pointer failed at %i", i);
        UNITY_TEST_ASSERT_EQUAL_INT(myInt, myItem[i * sizeof(int)], __LINE__, stringBuffer);
    }
}

TEST_CASE("List change item data", LIST_TAG) {
    // TODO: 09-Jul-2022 @basshelal: Item is added to list then changed here, should be reflected when calling list
    //  .get() including with NULL
}

TEST_CASE("List growth", LIST_TAG) {
    ListOptions *listOptions = list_defaultListOptions();
    listOptions->capacity = 1;
    listOptions->isGrowable = true;
    List *list = list_createOptions(listOptions);

    list_addItem(list, &myInt);

    UNITY_TEST_ASSERT_EQUAL_INT(1, list_size(list), __LINE__, "List size was incorrect");

    const int myOtherInt = 420;

    list_addItem(list, &myOtherInt);

    UNITY_TEST_ASSERT_EQUAL_INT(2, list_size(list), __LINE__, "List size was incorrect");

    UNITY_TEST_ASSERT_EQUAL_INT(myInt, *((int *) list_getItem(list, 0)), __LINE__, "");
    UNITY_TEST_ASSERT_EQUAL_INT(myOtherInt, *((int *) list_getItem(list, 1)), __LINE__, "");

    list_destroy(list);
}

TEST_CASE("List set and get items", LIST_TAG) {
    // TODO: 09-Jul-2022 @basshelal: Set and get items indexed and including NULL
}

TEST_CASE("List IndexOf", LIST_TAG) {
    // TODO: 09-Jul-2022 @basshelal: Duplicates and non-existent
}

TEST_CASE("List add item", LIST_TAG) {
    // TODO: 09-Jul-2022 @basshelal: Add item, add item indexed including NULL
}

TEST_CASE("List remove item", LIST_TAG) {
    // TODO: 09-Jul-2022 @basshelal: Remove item, add item indexed including NULL and non-existent and duplicate
}