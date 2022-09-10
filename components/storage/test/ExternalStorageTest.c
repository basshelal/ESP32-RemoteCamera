#include "Assertions.h"
#include "TestUtils.h"
#include "ExternalStorage.h"
#include <string.h>

beginTestFile("ExternalStorage")

// default external storage options
const ExternalStorageOptions externalStorageOptions = {.startAutoMountTask = false};

#define testDirName "testDir"
#define newTestDirName "newTestDir"

private inline char **createEmptyStringArray(const uint size) {
    char **array = alloc(sizeof(char *) * size);
    for (int i = 0; i < size; i++) {
        array[i] = NULL;
    }
    return array;
}

private inline void clearDir(const char *dirName) {
    bool dirExists;
    assertOK(externalStorage_queryDirExists(dirName, &dirExists));
    if (dirExists) {
        size_t entryCount;
        assertOK(externalStorage_readDir(dirName, NULL, &entryCount));
        char **dirEntries = malloc(entryCount * sizeof(char));
        assertOK(externalStorage_readDir(dirName, dirEntries, &entryCount));
        char buffer[EXTERNAL_STORAGE_MAX_PATH_LENGTH];
        for (int i = 0; i < entryCount; i++) {
            const char *entry = dirEntries[i];
            if (entry != NULL) {
                sprintf(buffer, "%s/%s", dirName, entry);
                assertOK(externalStorage_deleteFile(buffer));
            }
        }
        assertOK(externalStorage_readDir(dirName, NULL, &entryCount));
        assertEqualUInt(0, entryCount);
    }
}

private inline void fillDir(const char *dirName, const uint fileCount, const uint dirCount) {
    bool dirExists;
    externalStorage_queryDirExists(dirName, &dirExists);
    if (!dirExists) {
        assertOK(externalStorage_createDir(dirName));
        externalStorage_queryDirExists(dirName, &dirExists);
        assertTrue(dirExists);
    }
    clearDir(dirName);
    char buffer[EXTERNAL_STORAGE_MAX_PATH_LENGTH];
    for (int i = 0; i < fileCount; i++) {
        sprintf(buffer, "%s/%s%i", dirName, "testFile", i);
        assertOK(externalStorage_createFile(buffer));
    }
    for (int i = 0; i < dirCount; i++) {
        sprintf(buffer, "%s/%s%i", dirName, "testDir", i);
        assertOK(externalStorage_createDir(buffer));
    }
    size_t entryCount;
    assertOK(externalStorage_readDir(dirName, NULL, &entryCount));
    assertEqualUInt(fileCount + dirCount, entryCount);
}

private inline void ensureNonExistentDir(const char *dirName) {
    bool dirExists;
    externalStorage_queryDirExists(dirName, &dirExists);
    if (dirExists) {
        assertOK(externalStorage_deleteDir(dirName));
        externalStorage_queryDirExists(dirName, &dirExists);
        assertFalse(dirExists);
    }
}

testCase("Storage init and destroy") {
    assertOK(externalStorage_init(&externalStorageOptions));
    externalStorage_unmountSDCard(); // ok if failed
    assertOK(externalStorage_destroy());
}

testCase("re-init for following tests") {
    assertOK(externalStorage_init(&externalStorageOptions));
}

testCase("get StorageInfo") {
    StorageInfo storageInfo;
    assertOK(externalStorage_getStorageInfo(&storageInfo));
    assertNotNull(storageInfo.mountPoint);
    assertEqualUInt64(storageInfo.usedBytes + storageInfo.freeBytes, storageInfo.totalBytes);
}

testCase("non existent dir query, read, move, delete") {
    ensureNonExistentDir(testDirName); // ensure dir is non existent before tests

    bool dirExists = true;
    assertOK(externalStorage_queryDirExists(testDirName, &dirExists));
    assertFalse(dirExists);

    DirInfo dirInfo = {.sizeBytes = UINT64_MAX};
    assertEqualInt(STORAGE_ERROR_NOT_FOUND,
                   externalStorage_queryDirInfo(testDirName, &dirInfo));
    assertEqualUInt64(UINT64_MAX, dirInfo.sizeBytes); // no change if failed

    size_t entryCount = SIZE_MAX;
    assertEqualInt(STORAGE_ERROR_NOT_FOUND,
                   externalStorage_readDir(testDirName, NULL, &entryCount));
    assertEqualUInt(0, entryCount); // 0 only if dir wasn't found

    const uint testEntryCount = 16;
    char **dirEntries = createEmptyStringArray(testEntryCount);
    for (int i = 0; i < testEntryCount; i++) {
        assertNull(dirEntries[i]);
    }
    entryCount = SIZE_MAX;
    assertEqualInt(STORAGE_ERROR_NOT_FOUND,
                   externalStorage_readDir(testDirName, dirEntries, &entryCount));
    assertEqualUInt(0, entryCount); // 0 only if dir wasn't found
    for (int i = 0; i < testEntryCount; i++) {
        assertNull(dirEntries[i]);
    }
    free(dirEntries);

    assertEqualInt(STORAGE_ERROR_NOT_FOUND,
                   externalStorage_moveDir(testDirName, "newTestDir"));

    assertEqualInt(STORAGE_ERROR_NOT_FOUND,
                   externalStorage_deleteDir(testDirName));
}

testCase("dir create, query, read, move, delete") {
    ensureNonExistentDir(testDirName); // ensure dir is non existent before tests
    ensureNonExistentDir(newTestDirName); // ensure dir is non existent before tests

    assertOK(externalStorage_createDir(testDirName));

    bool dirExists = false;
    assertOK(externalStorage_queryDirExists(testDirName, &dirExists));
    assertTrue(dirExists);

    fillDir(testDirName, 5, 5);

    size_t entryCount = SIZE_MAX;
    assertOK(externalStorage_readDir(testDirName, NULL, &entryCount));
    assertEqualUInt(5 + 5, entryCount);

    char **dirEntries = createEmptyStringArray(entryCount);
    assertOK(externalStorage_readDir(testDirName, dirEntries, &entryCount));
    for (int i = 0; i < entryCount; i++) {
        char *entry = dirEntries[i];
        assertNotNull(entry);
        free(dirEntries[i]);
        dirEntries[i] = NULL;
    }
    free(dirEntries);

    assertOK(externalStorage_moveDir(testDirName, newTestDirName));

    assertOK(externalStorage_queryDirExists(testDirName, &dirExists));
    assertFalse(dirExists);

    assertOK(externalStorage_queryDirExists(newTestDirName, &dirExists));
    assertTrue(dirExists);

    assertOK(externalStorage_deleteDir(newTestDirName));

    assertOK(externalStorage_queryDirExists(newTestDirName, &dirExists));
    assertFalse(dirExists);
}

endTestFile