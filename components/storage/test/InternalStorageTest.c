#include <time.h>
#include <memory.h>
#include "unity.h"
#include "TestUtils.h"
#include "InternalStorage.h"
#include "StorageError.h"

// TODO: 21-Jul-2022 @basshelal: Add test cases to test for expected failures (functions that should fail)

#define TEST_TAG "[InternalStorage]"
#define TEST(name) TEST_CASE(name, TEST_TAG)
#define XTEST(name) XTEST_CASE(name, TEST_TAG)

XTEST("Internal storage init and destroy") {
    StorageError err = internalStorage_init();
    ASSERT(err == STORAGE_ERROR_NONE, "Error should be NONE but was: %s", storageError_toString(err));
    err = internalStorage_destroy();
    ASSERT(err == STORAGE_ERROR_NONE, "Error should be NONE but was: %s", storageError_toString(err));
}

XTEST("Internal storage file operations") {
    internalStorage_init();

    const char *filePath = "MyTestFile";
    StorageError err;
    bool exists;

    exists = internalStorage_queryFileExists(filePath);
    ASSERT(!exists, "File should not exist but does");

    const int textSize = 128;
    char originalText[textSize];
    sprintf(originalText, "The time is: %lu\n", clock());
    uint bytesWritten;
    err = internalStorage_writeFile(filePath, originalText, textSize, &bytesWritten);

    ASSERT_INT_EQUAL(STORAGE_ERROR_NONE, err, "writeFile() should not return error");
    ASSERT_INT_EQUAL(textSize, bytesWritten, "writeFile() written bytes were not equal");

    exists = internalStorage_queryFileExists(filePath);
    ASSERT(exists, "File should exist but does not");

    uint fileSize;
    err = internalStorage_queryFileSize(filePath, &fileSize);
    ASSERT_INT_EQUAL(STORAGE_ERROR_NONE, err, "queryFileSize() should not return error");
    ASSERT_INT_EQUAL(textSize, fileSize, "queryFileSize() bytes were not equal");

    char readText[textSize];
    uint bytesRead;
    err = internalStorage_readFile(filePath, readText, textSize, &bytesRead);
    ASSERT_INT_EQUAL(STORAGE_ERROR_NONE, err, "readFile() should not return error");
    ASSERT_INT_EQUAL(textSize, bytesRead, "readFile() bytes were not equal");
    ASSERT_STRING_EQUAL(originalText, readText, "Strings were not equal");

    err = internalStorage_deleteFile(filePath);
    ASSERT_INT_EQUAL(STORAGE_ERROR_NONE, err, "deleteFile() should not return error");

    exists = internalStorage_queryFileExists(filePath);
    ASSERT(!exists, "File should not exist but does");

    internalStorage_destroy();
}

XTEST("Internal storage key operations with String") {
    internalStorage_init();

    const char *key = "MyTestKey";
    StorageError err;
    bool exists;

    exists = internalStorage_hasKey(key);
    ASSERT(!exists, "Key should not exist but does");

    const int textSize = 128;
    char originalText[textSize];
    sprintf(originalText, "The time is: %lu\n", clock());
    err = internalStorage_putString(key, originalText);
    ASSERT_INT_EQUAL(STORAGE_ERROR_NONE, err, "putString() should not return error");

    char readText[textSize];
    err = internalStorage_getString(key, readText);
    ASSERT_INT_EQUAL(STORAGE_ERROR_NONE, err, "getString() should not return error");
    ASSERT_STRING_EQUAL(originalText, readText, "Strings were not equal");

    err = internalStorage_deleteKey(key);
    ASSERT_INT_EQUAL(STORAGE_ERROR_NONE, err, "deleteKey() should not return error");

    exists = internalStorage_hasKey(key);
    ASSERT(!exists, "Key should not exist but does");

    internalStorage_destroy();
}