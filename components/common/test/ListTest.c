#include <memory.h>
#include "List.h"
#include "unity.h"
#include "TestUtils.h"

// TODO: 09-Jul-2022 @basshelal: Cleaner smaller tests, use macro helpers if needed

#define TAG "[List]"

int myInt = 69;

TEST_CASE("Create List", TAG) {
    List *list = list_create();
    ASSERT_NOT_NULL(list, "List should be non-null");
    list_destroy(list);
}

TEST_CASE("Create List with Options Capacity and isGrowable", TAG) {
    ListOptions listOptions = {
            .capacity = 1,
            .isGrowable = false,
            .growthFactor = 1,
            .errorCallback = NULL
    };
    List *list = list_createOptions(&listOptions);
    ASSERT_NOT_NULL(list, "List should be non-null");

    list_addItem(list, &myInt);
    list_addItem(list, &myInt);
    list_addItem(list, &myInt);

    ASSERT_INT_EQUAL(listOptions.capacity, list_size(list), "List size should not be larger than it's capacity");
    list_destroy(list);
}

TEST_CASE("Create List with Options growth factor", TAG) {
    ListOptions listOptions = {
            .capacity = 1,
            .isGrowable = true,
            .growthFactor = 2,
            .errorCallback = NULL
    };
    List *list = list_createOptions(&listOptions);
    ASSERT_NOT_NULL(list, "List should be non-null");

    list_addItem(list, &myInt);
    list_addItem(list, &myInt);

    ASSERT_INT_EQUAL(2, list_size(list), "List size should be 2");
    list_destroy(list);
}

int callbackCalled = 0;

void error_callback(const ListError listError) {
    callbackCalled++;
}

TEST_CASE("Create List with Options error callback", TAG) {
    ListOptions listOptions = {
            .capacity = 1,
            .isGrowable = false,
            .growthFactor = 2,
            .errorCallback = error_callback
    };
    List *list = list_createOptions(&listOptions);
    ASSERT_NOT_NULL(list, "List should be non-null");

    list_addItem(list, &myInt);
    list_addItem(list, &myInt);

    ASSERT_INT_GREATER_THAN(0, callbackCalled, "Error callback should have been called");
    list_destroy(list);
    callbackCalled = 0;
}

TEST_CASE("List Size", TAG) {
    ASSERT_INT_EQUAL(-1, list_size(NULL), "Null list size should be -1");

    List *list = list_create();

    const int mySize = 100;
    for (int i = 0; i < mySize; i++) {
        list_addItem(list, &myInt);
    }

    ASSERT_INT_EQUAL(mySize, list_size(list), "List size was incorrect");

    list_destroy(list);
}

TEST_CASE("List isEmpty", TAG) {
    ASSERT_INT_EQUAL(true, list_isEmpty(NULL), "Null List isEmpty should be true");

    List *list = list_create();

    ASSERT_INT_EQUAL(true, list_isEmpty(list), "List isEmpty should be true");

    list_addItem(list, &myInt);

    ASSERT_INT_EQUAL(false, list_isEmpty(list), "List isEmpty should be false");

    list_destroy(list);
}

TEST_CASE("List get item", TAG) {
    List *list = list_create();

    list_addItem(list, &myInt);

    int *result = (int *) list_getItem(list, 0);

    ASSERT_NOT_NULL(result, "Result should not be null");

    ASSERT_INT_EQUAL(myInt, *result, "List get item was incorrect");

    list_destroy(list);
}

TEST_CASE("Access items after list destroy", TAG) {
    List *list = list_create();

    const int itemCount = 128;
    const int size = (int) (sizeof(int) * itemCount);
    int *myItem = calloc(1, size);
    for (int i = 0; i < itemCount; i++) {
        myItem[i * sizeof(int)] = myInt;
    }

    list_addItem(list, myItem);

    ASSERT_MEMORY_EQUAL(myItem, list_getItem(list, 0), size, "Items should be identical");

    list_destroy(list);

    list = NULL;

    ASSERT_NOT_NULL(myItem, "Pointer should not be null");

    char stringBuffer[128];
    for (int i = 0; i < itemCount; i++) {
        snprintf(stringBuffer, 128, "Pointer failed at %i", i);
        ASSERT_INT_EQUAL(myInt, myItem[i * sizeof(int)], "%s", stringBuffer);
    }
}

TEST_CASE("List change item data", TAG) {
    // TODO: 09-Jul-2022 @basshelal: Item is added to list then changed here, should be reflected when calling list
    //  .get() including with NULL
}

TEST_CASE("List growth", TAG) {
    ListOptions *listOptions = list_defaultListOptions();
    listOptions->capacity = 1;
    listOptions->isGrowable = true;
    List *list = list_createOptions(listOptions);

    list_addItem(list, &myInt);

    ASSERT_INT_EQUAL(1, list_size(list), "List size was incorrect");

    const int myOtherInt = 420;

    list_addItem(list, &myOtherInt);

    ASSERT_INT_EQUAL(2, list_size(list), "List size was incorrect");

    ASSERT_INT_EQUAL(myInt, *((int *) list_getItem(list, 0)), "Item was incorrect");
    ASSERT_INT_EQUAL(myOtherInt, *((int *) list_getItem(list, 1)), "Item was incorrect");

    list_destroy(list);
}

TEST_CASE("List set and get items", TAG) {
    // TODO: 09-Jul-2022 @basshelal: Set and get items indexed and including NULL
}

TEST_CASE("List IndexOf", TAG) {
    // TODO: 09-Jul-2022 @basshelal: Duplicates and non-existent
}

TEST_CASE("List add item", TAG) {
    // TODO: 09-Jul-2022 @basshelal: Add item, add item indexed including NULL
}

TEST_CASE("List remove item", TAG) {
    // TODO: 09-Jul-2022 @basshelal: Remove item, add item indexed including NULL and non-existent and duplicate
}