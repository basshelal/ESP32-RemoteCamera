#include "unity.h"
#include "List.h"
#include "TestUtils.h"
#include "esp_heap_caps.h"

#define TEST_TAG "[List]"
#define TEST(name) TEST_CASE(name, TEST_TAG)
#define XTEST(name) XTEST_CASE(name, TEST_TAG)

#define listCapacity(list) *((capacity_t *) list)

int myTestItems[10] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512};

// TODO: 07-Aug-2022 @basshelal: These could be cleaned up better, also error checking tests could be added
//  but so far this is the minimal required

TEST("List create") {
    List *list = list_create();
    ASSERT_NOT_NULL(list, "List should not be null");
    list_destroy(list);
}

TEST("List create with options capacity and isGrowable") {
    ListOptions listOptions = {
            .capacity = 1,
            .isGrowable = false,
            .growthFactor = 1,
            .errorCallback = NULL
    };
    List *list = list_createWithOptions(&listOptions);
    ASSERT_NOT_NULL(list, "List should not be null");

    list_addItem(list, &myTestItems[0]);
    list_addItem(list, &myTestItems[1]);
    list_addItem(list, &myTestItems[2]);

    ASSERT_UINT_EQUAL(listOptions.capacity, list_getSize(list), "List size should not be larger than its capacity");
    list_destroy(list);
}

TEST("List create with options growthFactor") {
    ListOptions listOptions = {
            .capacity = 1,
            .isGrowable = true,
            .growthFactor = 2,
            .errorCallback = NULL
    };
    List *list = list_createWithOptions(&listOptions);
    ASSERT_NOT_NULL(list, "List should not be null");

    list_addItem(list, &myTestItems[0]);
    list_addItem(list, &myTestItems[1]);

    ASSERT_UINT_EQUAL(2, list_getSize(list), "List size should be 2");
    list_destroy(list);
}

int callbackCalled = 0;

void error_callback(const Error listError) {
    callbackCalled++;
}

TEST("List create with options errorCallback") {
    ListOptions listOptions = {
            .capacity = 1,
            .isGrowable = false,
            .growthFactor = 1,
            .errorCallback = error_callback
    };
    List *list = list_createWithOptions(&listOptions);
    ASSERT_NOT_NULL(list, "List should not be null");

    list_addItem(list, &myTestItems[0]);
    list_addItem(list, &myTestItems[1]);

    ASSERT_INT_EQUAL(1, callbackCalled, "Error callback should have been called exactly once");
    list_destroy(list);
    callbackCalled = 0;
}

TEST("List Size") {
    ASSERT_UINT_EQUAL(LIST_INVALID_INDEX_CAPACITY, list_getSize(NULL), "Null list size should be invalid");

    List *list = list_create();
    ASSERT_UINT_EQUAL(0, list_getSize(list), "List size should be 0");

    const unsigned int size = 256;
    char testItems[size];
    for (int i = 0; i < size; i++) {
        testItems[i] = (char) i;
    }
    for (int i = 0; i < size; i++) {
        list_addItem(list, &testItems[i]);
    }

    ASSERT_UINT_EQUAL(size, list_getSize(list), "List size was incorrect");
    list_clear(list);
    ASSERT_UINT_EQUAL(0, list_getSize(list), "List size should be 0");

    list_destroy(list);
}

TEST("List isEmpty") {
    ASSERT(list_isEmpty(NULL), "Null List isEmpty should be true");

    List *list = list_create();
    ASSERT(list_isEmpty(list), "List isEmpty should be true");

    const unsigned int size = 256;
    char testItems[size];
    for (int i = 0; i < size; i++) {
        testItems[i] = (char) i;
    }
    for (int i = 0; i < size; i++) {
        list_addItem(list, &testItems[i]);
    }

    ASSERT_FALSE(list_isEmpty(list), "List isEmpty should be false");

    list_clear(list);
    ASSERT(list_isEmpty(list), "List isEmpty should be true");

    list_destroy(list);
}

TEST("List addItem") {
    List *list = list_create();

    char myString[16];
    sprintf(myString, "This is a test");
    bool myBool = false;

    list_addItem(list, &myTestItems[0]);
    list_addItem(list, NULL);
    list_addItem(list, myString);
    list_addItem(list, &myBool);

    ASSERT_INT_EQUAL(myTestItems[0], *((int *) list_getItem(list, 0)), "item at index 0 was incorrect");
    ASSERT_NULL(list_getItem(list, 1), "item at index 1 should be NULL");
    ASSERT_STRING_EQUAL(myString, list_getItem(list, 2), "strings were not equal");
    ASSERT_FALSE(*((bool *) list_getItem(list, 3)), "item at index 3 should be false");

    list_destroy(list);
}

TEST("List addItemIndexed") {
    List *list = list_create();

    list_addItemIndexed(list, 0, &myTestItems[0]);
    int *result = list_getItem(list, 0);
    ASSERT_NOT_NULL(result, "result should not be null");
    ASSERT_INT_EQUAL(myTestItems[0], *result, "items were not equal");

    list_addItem(list, &myTestItems[1]);
    list_addItem(list, &myTestItems[2]);
    ASSERT_UINT_EQUAL(3, list_getSize(list), "size was not correct");

    list_addItemIndexed(list, 0, NULL);
    ASSERT_NULL(list_getItem(list, 0), "result should be null");
    ASSERT_UINT_EQUAL(4, list_getSize(list), "size was not correct");
    result = list_getItem(list, 1);
    ASSERT_NOT_NULL(result, "result should not be null");
    ASSERT_INT_EQUAL(myTestItems[0], *result, "items were not equal");

    list_destroy(list);
}

TEST("List getItem") {
    List *list = list_create();

    const unsigned int size = 256;
    char testItems[size];
    for (int i = 0; i < size; i++) {
        testItems[i] = (char) i;
    }
    for (int i = 0; i < size; i++) {
        list_addItem(list, &testItems[i]);
    }
    ASSERT_UINT_EQUAL(size, list_getSize(list), "List size was incorrect");

    for (int i = 0; i < size; i++) {
        ListItem *result = list_getItem(list, i);
        ASSERT_NOT_NULL(result, "Result should not be null");
        ASSERT_INT_EQUAL(testItems[i], *((char *) result), "List getItem returned incorrect item");
    }

    list_destroy(list);
}

TEST("List growth") {
    ListOptions listOptions = LIST_DEFAULT_OPTIONS;
    listOptions.capacity = 2;
    listOptions.isGrowable = true;
    listOptions.growthFactor = 2;
    List *list = list_createWithOptions(&listOptions);

    // reflection-like access based on known ListData internals
    capacity_t capacity = listCapacity(list);
    ASSERT_UINT_EQUAL(listOptions.capacity, capacity, "Capacity was incorrect");

    list_addItem(list, &myTestItems[0]);
    list_addItem(list, &myTestItems[1]);

    ASSERT_UINT_EQUAL(2, list_getSize(list), "List size was incorrect");

    list_addItem(list, &myTestItems[2]);

    ASSERT_INT_EQUAL(3, list_getSize(list), "List size was incorrect");

    capacity_t oldCapacity = capacity;
    capacity = listCapacity(list);
    ASSERT(capacity != oldCapacity, "Capacity should have changed but did not");
    ASSERT_UINT_EQUAL(listOptions.capacity * listOptions.growthFactor, capacity, "Capacity was incorrect");

    ASSERT_INT_EQUAL(myTestItems[0], *((int *) list_getItem(list, 0)), "Item was incorrect");
    ASSERT_INT_EQUAL(myTestItems[1], *((int *) list_getItem(list, 1)), "Item was incorrect");

    list_destroy(list);
}

TEST("List modify items") {
    List *list = list_create();

    int *myInt = malloc(sizeof(int));
    *myInt = myTestItems[0];

    list_setItem(list, 0, myInt);

    int *result = (int *) list_getItem(list, 0);
    ASSERT_NOT_NULL(result, "result should not be NULL");
    ASSERT_INT_EQUAL(myTestItems[0], *result, "List getItem items were not equal");

    *myInt = myTestItems[9];
    result = (int *) list_getItem(list, 0);
    ASSERT_NOT_NULL(result, "result should not be NULL");
    ASSERT_INT_EQUAL(myTestItems[9], *result, "List getItem items were not equal");

    int **myIntReference = &myInt;
    list_setItem(list, 0, myIntReference);
    int **resultReference = (int **) list_getItem(list, 0);
    ASSERT_NOT_NULL(*resultReference, "result reference should not be null");
    *myIntReference = NULL;
    resultReference = (int **) list_getItem(list, 0);
    ASSERT_NULL(*resultReference, "result reference should be null");

    free(myInt);
    list_destroy(list);
}

TEST("List setItem") {
    List *list = list_create();

    Error err = list_setItem(list, 0, &myTestItems[0]);
    ASSERT(err == 0, "Expected no errors but actually was: %i", err);
    int *result = list_getItem(list, 0);
    ASSERT_NOT_NULL(result, "result should not be null");
    ASSERT_INT_EQUAL(myTestItems[0], *result, "List item at index 0 was incorrect");

    list_addItem(list, &myTestItems[1]);
    list_addItem(list, &myTestItems[2]);
    list_addItem(list, &myTestItems[3]);

    list_setItem(list, 2, &myTestItems[9]);
    result = list_getItem(list, 2);
    ASSERT_NOT_NULL(result, "result should not be null");
    ASSERT_INT_EQUAL(myTestItems[9], *result, "List items at index %i were not equal", 2);

    list_setItem(list, 0, NULL);
    result = list_getItem(list, 0);
    ASSERT_NULL(result, "result should be NULL");

    list_destroy(list);
}

TEST("List indexOfItem") {
    List *list = list_create();

    list_addItem(list, NULL);
    list_addItem(list, &myTestItems[0]);
    list_addItem(list, &myTestItems[1]);
    list_addItem(list, &myTestItems[2]);
    list_addItem(list, &myTestItems[2]); // intentional duplicate

    index_t index = list_indexOfItem(list, NULL);
    ASSERT_UINT_EQUAL(0, index, "index was incorrect");

    index = list_indexOfItem(list, &myTestItems[2]);
    ASSERT_UINT_EQUAL(3, index, "index was incorrect");

    index = list_indexOfItem(list, &myTestItems[9]);
    ASSERT_UINT_EQUAL(LIST_INVALID_INDEX_CAPACITY, index, "index was incorrect");

    list_destroy(list);
}

static bool equalityAlwaysTrue(const ListItem *a, const ListItem *b) {
    return true;
}

static bool equalityAlwaysFalse(const ListItem *a, const ListItem *b) {
    return false;
}

TEST("List indexOfItemFunction") {
    List *list = list_create();

    list_addItem(list, NULL);
    list_addItem(list, &myTestItems[0]);
    list_addItem(list, &myTestItems[1]);
    list_addItem(list, &myTestItems[2]);
    list_addItem(list, &myTestItems[2]); // intentional duplicate

    index_t index = list_indexOfItemFunction(list, NULL, equalityAlwaysTrue);
    ASSERT_UINT_EQUAL(0, index, "index was incorrect");
    index = list_indexOfItemFunction(list, &myTestItems[9], equalityAlwaysTrue);
    ASSERT_UINT_EQUAL(0, index, "index was incorrect");

    index = list_indexOfItemFunction(list, NULL, equalityAlwaysFalse);
    ASSERT_UINT_EQUAL(LIST_INVALID_INDEX_CAPACITY, index, "index was incorrect");
    index = list_indexOfItemFunction(list, &myTestItems[9], equalityAlwaysFalse);
    ASSERT_UINT_EQUAL(LIST_INVALID_INDEX_CAPACITY, index, "index was incorrect");

    list_destroy(list);
}

TEST("List removeItem") {
    List *list = list_create();

    list_addItem(list, NULL);
    list_addItem(list, &myTestItems[0]);
    list_addItem(list, &myTestItems[1]);
    list_addItem(list, &myTestItems[2]);
    list_addItem(list, &myTestItems[2]); // intentional duplicate

    ASSERT_UINT_EQUAL(5, list_getSize(list), "size was incorrect");

    list_removeItem(list, NULL);
    ASSERT_UINT_EQUAL(4, list_getSize(list), "size was incorrect");
    int *result = list_getItem(list, 0);
    ASSERT_INT_EQUAL(myTestItems[0], *result, "items were not equal");

    list_removeItem(list, &myTestItems[9]);
    ASSERT_UINT_EQUAL(4, list_getSize(list), "size was incorrect");

    list_removeItem(list, &myTestItems[2]);
    ASSERT_UINT_EQUAL(3, list_getSize(list), "size was incorrect");
    result = list_getItem(list, 2);
    ASSERT_INT_EQUAL(myTestItems[2], *result, "items were not equal");

    list_destroy(list);
}

TEST("List removeItemIndexed") {
    List *list = list_create();

    list_addItem(list, NULL);
    list_addItem(list, &myTestItems[0]);
    list_addItem(list, &myTestItems[1]);
    list_addItem(list, &myTestItems[2]);
    list_addItem(list, &myTestItems[2]); // intentional duplicate

    ASSERT_UINT_EQUAL(5, list_getSize(list), "size was incorrect");

    list_removeItemIndexed(list, 0);
    ASSERT_UINT_EQUAL(4, list_getSize(list), "size was incorrect");
    int *result = list_getItem(list, 0);
    ASSERT_INT_EQUAL(myTestItems[0], *result, "items were not equal");

    list_removeItemIndexed(list, 3);
    ASSERT_UINT_EQUAL(3, list_getSize(list), "size was incorrect");
    result = list_getItem(list, 2);
    ASSERT_INT_EQUAL(myTestItems[2], *result, "items were not equal");

    list_destroy(list);
}

TEST("List not shrinkable then manual shrink") {
    ListOptions listOptions = LIST_DEFAULT_OPTIONS;
    listOptions.isShrinkable = false;

    List *list = list_createWithOptions(&listOptions);

    for (int i = 0; i < 10; i++) {
        list_addItem(list, &myTestItems[i]);
    }
    ASSERT_INT_EQUAL(10, list_getSize(list), "List size was incorrect");

    int indicesToRemove[4] = {0, 4, 5, 9};

    for (int i = 0; i < 4; i++) {
        int index = indicesToRemove[i];
        list_removeItemIndexed(list, index);
        ASSERT_INT_EQUAL(10, list_getSize(list), "List size was incorrect");
        ASSERT_NULL(list_getItem(list, index), "Item at %i should be NULL", index);
    }

    list_shrink(list);

    int myTestItemsShrunk[6] = {2, 4, 8, 64, 128, 256};
    for (int i = 0; i < 6; i++) {
        ASSERT_INT_EQUAL(myTestItemsShrunk[i], *((int *) list_getItem(list, i)), "Items were not equal at %i", i);
    }

    list_destroy(list);
}
