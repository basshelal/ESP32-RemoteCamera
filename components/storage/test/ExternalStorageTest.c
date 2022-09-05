#include "Assertions.h"
#include "TestUtils.h"
#include "ExternalStorage.h"
#include <ffconf.h>
#include <string.h>

beginTestFile("ExternalStorage")

// default external storage options
const ExternalStorageOptions externalStorageOptions = {.startAutoMountTask = false};

#define testDirName "testDir"

private inline char **createEmptyStringArray(const uint size) {
    char **array = alloc(sizeof(char *) * size);
    for (int i = 0; i < size; i++) {
        array[i] = NULL;
    }
    return array;
}

private inline void clearTestDir(const char *dirName) {
    bool dirExists;
    externalStorage_queryDirExists(dirName, &dirExists);
    if (dirExists) {
        size_t entryCount;
        externalStorage_readDir(dirName, NULL, &entryCount);
        char **dirEntries = malloc(entryCount * sizeof(char));
        externalStorage_readDir(dirName, dirEntries, &entryCount);
        char buffer[FF_MAX_LFN];
        for (int i = 0; i < entryCount; i++) {
            const char *entry = dirEntries[i];
            if (entry != NULL) {
                sprintf(buffer, "%s/%s", dirName, entry);
                externalStorage_deleteFile(buffer);
            }
        }
        externalStorage_readDir(dirName, NULL, &entryCount);
        assertEqualUInt(0, entryCount);
    }
}

private inline void fillTestDir(const char *dirName, const uint fileCount, const uint dirCount) {
    externalStorage_createDir(dirName);
    clearTestDir(dirName);
    char buffer[FF_MAX_LFN];
    for (int i = 0; i < fileCount; i++) {
        sprintf(buffer, "%s/%s%i", dirName, "testFile", i);
        externalStorage_createFile(buffer);
    }
    for (int i = 0; i < dirCount; i++) {
        sprintf(buffer, "%s/%s%i", dirName, "testDir", i);
        externalStorage_createFile(buffer);
    }
    size_t entryCount;
    externalStorage_readDir(dirName, NULL, &entryCount);
    assertEqualUInt(fileCount + dirCount, entryCount);
}

testCase("Storage init and destroy") {
    assertOK(externalStorage_init(&externalStorageOptions));
    externalStorage_unmountSDCard();
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
    externalStorage_deleteDir(testDirName);

    bool dirExists = true;
    assertOK(externalStorage_queryDirExists(testDirName, &dirExists));
    assertFalse(dirExists);

    DirInfo dirInfo = {.sizeBytes = UINT64_MAX};
    assertEqualInt(STORAGE_ERROR_NOT_FOUND,
                   externalStorage_queryDirInfo(testDirName, &dirInfo));
    assertEqualUInt64(UINT64_MAX, dirInfo.sizeBytes);

    size_t entryCount = SIZE_MAX;
    assertEqualInt(STORAGE_ERROR_NOT_FOUND,
                   externalStorage_readDir(testDirName, NULL, &entryCount));
    assertEqualUInt(0, entryCount);

    const uint testEntryCount = 16;
    char **dirEntries = createEmptyStringArray(testEntryCount);
    for (int i = 0; i < testEntryCount; i++) {
        assertNull(dirEntries[i]);
    }
    entryCount = SIZE_MAX;
    assertEqualInt(STORAGE_ERROR_NOT_FOUND,
                   externalStorage_readDir(testDirName, dirEntries, &entryCount));
    assertEqualUInt(0, entryCount);
    for (int i = 0; i < testEntryCount; i++) {
        assertNull(dirEntries[i]);
    }
    free(dirEntries);

    assertEqualInt(STORAGE_ERROR_NOT_FOUND,
                   externalStorage_moveDir(testDirName, "newTestDir"));

    assertEqualInt(STORAGE_ERROR_NOT_FOUND,
                   externalStorage_deleteDir(testDirName));
}

testCase("dir create ,query, move, delete") {
    externalStorage_deleteDir(testDirName);

    assertOK(externalStorage_createDir(testDirName));
}

testCase("read dir") {

    size_t entryCount = SIZE_MAX;
    assertOK(externalStorage_readDir(testDirName, NULL, &entryCount));
    assertTrue(entryCount != SIZE_MAX);

    char **dirEntries = createEmptyStringArray(entryCount);
    assertOK(externalStorage_readDir(testDirName, dirEntries, &entryCount));
    for (int i = 0; i < entryCount; i++) {
        char *entry = dirEntries[i];
        printf("%i: %s\n", i, entry);
        assertNotNull(entry);
    }
    for (int i = 0; i < entryCount; i++) {
        free(dirEntries[i]);
        dirEntries[i] = NULL;
    }

    free(dirEntries);
}

endTestFile