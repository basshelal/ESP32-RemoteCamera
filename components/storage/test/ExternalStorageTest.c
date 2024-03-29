#include "Assertions.h"
#include "TestUtils.h"
#include "ExternalStorage.h"
#include <string.h>

beginTestFile("ExternalStorage")

// default external storage options
const ExternalStorageOptions externalStorageOptions = {.startAutoMountTask = false};

#define testDirName "testDir"
#define newTestDirName "newTestDir"
#define testFileName "testFile"
#define newTestFileName "newTestFile"

private inline char **createEmptyStringArray(const uint size) {
    char **array = alloc(sizeof(char *) * size);
    for (int i = 0; i < size; i++) {
        array[i] = NULL;
    }
    return array;
}

private inline void clearDir(const char *dirPath) {
    bool dirExists;
    assertOK(externalStorage_queryDirExists(dirPath, &dirExists));
    if (dirExists) {
        size_t entryCount;
        assertOK(externalStorage_readDir(dirPath, NULL, &entryCount));
        char **dirEntries = alloc(entryCount * sizeof(char *));
        assertOK(externalStorage_readDir(dirPath, dirEntries, &entryCount));
        char buffer[EXTERNAL_STORAGE_MAX_PATH_LENGTH];
        for (int i = 0; i < entryCount; i++) {
            const char *entry = dirEntries[i];
            if (entry != NULL) {
                sprintf(buffer, "%s/%s", dirPath, entry);
                assertOK(externalStorage_deleteFile(buffer));
            }
        }
        assertOK(externalStorage_readDir(dirPath, NULL, &entryCount));
        assertEqualUInt(0, entryCount);
    }
}

private inline void fillDir(const char *dirPath, const uint fileCount, const uint dirCount) {
    bool dirExists;
    externalStorage_queryDirExists(dirPath, &dirExists);
    if (!dirExists) {
        assertOK(externalStorage_createDir(dirPath));
        externalStorage_queryDirExists(dirPath, &dirExists);
        assertTrue(dirExists);
    }
    clearDir(dirPath);
    char buffer[EXTERNAL_STORAGE_MAX_PATH_LENGTH];
    for (int i = 0; i < fileCount; i++) {
        sprintf(buffer, "%s/%s%i", dirPath, "testFile", i);
        assertOK(externalStorage_createFile(buffer));
    }
    for (int i = 0; i < dirCount; i++) {
        sprintf(buffer, "%s/%s%i", dirPath, "testDir", i);
        assertOK(externalStorage_createDir(buffer));
    }
    size_t entryCount;
    assertOK(externalStorage_readDir(dirPath, NULL, &entryCount));
    assertEqualUInt(fileCount + dirCount, entryCount);
}

private inline void ensureNonExistentDir(const char *dirPath) {
    bool dirExists;
    externalStorage_queryDirExists(dirPath, &dirExists);
    if (dirExists) {
        assertOK(externalStorage_deleteDir(dirPath));
        externalStorage_queryDirExists(dirPath, &dirExists);
        assertFalse(dirExists);
    }
}

private inline void ensureNonExistentFile(const char *filePath) {
    bool fileExists;
    externalStorage_queryFileExists(filePath, &fileExists);
    if (fileExists) {
        assertOK(externalStorage_deleteFile(filePath));
        externalStorage_queryFileExists(filePath, &fileExists);
        assertFalse(fileExists);
    }
}

private inline void ensureDir(const char *dirPath) {
    bool dirExists;
    assertOK(externalStorage_queryDirExists(dirPath, &dirExists));
    if (!dirExists) {
        assertOK(externalStorage_createDir(dirPath));
        assertOK(externalStorage_queryDirExists(dirPath, &dirExists));
        assertTrue(dirExists);
    }
}

private inline void ensureFile(const char *filePath) {
    bool fileExists;
    assertOK(externalStorage_queryFileExists(filePath, &fileExists));
    if (!fileExists) {
        assertOK(externalStorage_createFile(filePath));
        assertOK(externalStorage_queryFileExists(filePath, &fileExists));
        assertTrue(fileExists);
    }
}

testCase("Storage init and destroy") {
    assertOK(externalStorage_init(&externalStorageOptions));
    externalStorage_unmountSDCard(); // ok if failed
    assertOK(externalStorage_destroy());
}

run("init for following tests") {
    assertOK(externalStorage_init(&externalStorageOptions));
}

testCase("get StorageInfo") {
    StorageInfo storageInfo;
    assertOK(externalStorage_getStorageInfo(&storageInfo));
    assertNotNull(storageInfo.mountPoint);
    assertEqualUInt(storageInfo.usedBytes + storageInfo.freeBytes, storageInfo.totalBytes);
}

run("ensure dir is non-existent for following tests") {
    ensureNonExistentDir(testDirName);
}

testCase("non existent dir query exists") {
    bool dirExists = true;
    assertOK(externalStorage_queryDirExists(testDirName, &dirExists));
    assertFalse(dirExists);
}

testCase("non existent dir query info") {
    DirInfo dirInfo = {.sizeBytes = UINT_MAX};
    assertEqualInt(ERROR_NOT_FOUND,
                   externalStorage_queryDirInfo(testDirName, &dirInfo));
    assertEqualUInt(UINT_MAX, dirInfo.sizeBytes); // no change if failed
}

testCase("non existent dir read") {
    size_t entryCount = SIZE_MAX;
    assertEqualInt(ERROR_NOT_FOUND,
                   externalStorage_readDir(testDirName, NULL, &entryCount));
    assertEqualUInt(0, entryCount); // 0 only if dir wasn't found

    const uint testEntryCount = 16;
    char **dirEntries = createEmptyStringArray(testEntryCount);
    for (int i = 0; i < testEntryCount; i++) {
        assertNull(dirEntries[i]);
    }
    entryCount = SIZE_MAX;
    assertEqualInt(ERROR_NOT_FOUND,
                   externalStorage_readDir(testDirName, dirEntries, &entryCount));
    assertEqualUInt(0, entryCount); // 0 only if dir wasn't found
    for (int i = 0; i < testEntryCount; i++) {
        assertNull(dirEntries[i]);
    }
    free(dirEntries);
}

testCase("non existent dir move") {
    assertEqualInt(ERROR_NOT_FOUND,
                   externalStorage_moveDir(testDirName, "newTestDir"));
}

testCase("non existent dir delete") {
    assertEqualInt(ERROR_NOT_FOUND,
                   externalStorage_deleteDir(testDirName));
}

run("ensure dir is non-existent for following tests") {
    ensureNonExistentDir(testDirName);
    ensureNonExistentDir(newTestDirName);
}

testCase("dir create") {
    assertOK(externalStorage_createDir(testDirName));
}

testCase("dir query exists") {
    bool dirExists = false;
    assertOK(externalStorage_queryDirExists(testDirName, &dirExists));
    assertTrue(dirExists);
}

testCase("dir query info") {
    // TODO: 11-Sep-2022 @basshelal: We might delete this, otherwise implement it
    ignoreTest();
}

testCase("dir read") {
    const uint fileCount = 5;
    const uint dirCount = 5;

    fillDir(testDirName, fileCount, dirCount);

    size_t entryCount = SIZE_MAX;
    assertOK(externalStorage_readDir(testDirName, NULL, &entryCount));
    assertEqualUInt(fileCount + dirCount, entryCount);

    char **dirEntries = createEmptyStringArray(entryCount);
    assertOK(externalStorage_readDir(testDirName, dirEntries, &entryCount));
    for (int i = 0; i < entryCount; i++) {
        char *entry = dirEntries[i];
        assertNotNull(entry);
        free(dirEntries[i]);
        dirEntries[i] = NULL;
    }
    free(dirEntries);
}

testCase("dir move") {
    assertOK(externalStorage_moveDir(testDirName, newTestDirName));

    bool dirExists = true;
    assertOK(externalStorage_queryDirExists(testDirName, &dirExists));
    assertFalse(dirExists);

    assertOK(externalStorage_queryDirExists(newTestDirName, &dirExists));
    assertTrue(dirExists);
}

testCase("dir delete") {
    assertOK(externalStorage_deleteDir(newTestDirName));

    bool dirExists = true;
    assertOK(externalStorage_queryDirExists(newTestDirName, &dirExists));
    assertFalse(dirExists);
}

run("ensure file is non-existent for following tests") {
    ensureNonExistentFile(testFileName);
    ensureNonExistentFile(newTestFileName);
}

testCase("non existent file query exists") {
    bool fileExists = true;
    assertOK(externalStorage_queryFileExists(testFileName, &fileExists));
    assertFalse(fileExists);
}

testCase("non existent file query info") {
    FileInfo fileInfo;
    fileInfo.sizeBytes = UINT_MAX;
    assertEqualInt(ERROR_NOT_FOUND,
                   externalStorage_queryFileInfo(testFileName, &fileInfo));
    assertEqualUInt(UINT_MAX, fileInfo.sizeBytes); // no change if failed
}

testCase("non existent file open") {
    FILE *file;
    assertEqualInt(ERROR_NOT_FOUND,
                   externalStorage_openFile(testFileName, &file, FILE_MODE_WRITE));
}

testCase("non existent file close") {
    FILE *file = NULL;
    assertEqualInt(ERROR_NULL_ARGUMENT,
                   externalStorage_closeFile(file));
}

testCase("non existent file read") {
    FILE *file = NULL;
    const uint bufferSize = 512;
    char *buffer = calloc(bufferSize, sizeof(char));
    for (int i = 0; i < bufferSize; i++) {
        assertEqualChar(0, buffer[i]);
    }
    uint bytesRead = UINT_MAX;
    assertEqualInt(ERROR_NULL_ARGUMENT,
                   externalStorage_readFile(file, /*startPosition=*/0, buffer, bufferSize, &bytesRead));
    for (int i = 0; i < bufferSize; i++) {
        assertEqualChar(0, buffer[i]); // no change if failed
    }
    assertEqualUInt(UINT_MAX, bytesRead); // no change if failed
    free(buffer);
}

testCase("non existent file write") {
    FILE *file = NULL;
    const uint bufferSize = 512;
    char *buffer = calloc(bufferSize, sizeof(char));
    uint bytesWritten = UINT_MAX;
    assertEqualInt(ERROR_NULL_ARGUMENT,
                   externalStorage_writeFile(file, /*startPosition=*/0, buffer, bufferSize, &bytesWritten));
    assertEqualUInt(UINT_MAX, bytesWritten); // no change if failed
    free(buffer);
}

testCase("non existent file move") {
    assertEqualInt(ERROR_NOT_FOUND,
                   externalStorage_moveFile(testFileName, newTestFileName));
}

testCase("non existent file delete") {
    assertEqualInt(ERROR_NOT_FOUND,
                   externalStorage_deleteFile(testFileName));
}

run("ensure file is non-existent for following tests") {
    ensureNonExistentFile(testFileName);
    ensureNonExistentFile(newTestFileName);
}

testCase("file create") {
    assertOK(externalStorage_createFile(testFileName));
}

testCase("file query exists") {
    bool fileExists = false;
    assertOK(externalStorage_queryFileExists(testFileName, &fileExists));
    assertTrue(fileExists);
}

testCase("file query info") {
    FileInfo fileInfo;
    fileInfo.sizeBytes = UINT_MAX;
    assertOK(externalStorage_queryFileInfo(testFileName, &fileInfo));
    assertNotEqualUInt(UINT_MAX, fileInfo.sizeBytes);
}

private FILE *file;

testCase("file open") {
    assertOK(externalStorage_openFile(testFileName, &file, FILE_MODE_WRITE));
    assertNotNull(file);
}

testCase("file close") {
    assertOK(externalStorage_closeFile(file));
}

private const uint bufferLength = 256;
private char *writeBuffer = NULL;

testCase("file write") {
    writeBuffer = calloc(bufferLength, sizeof(char));
    assertNotNull(writeBuffer);
    for (int i = 0; i < bufferLength; i++) {
        writeBuffer[i] = (char) i;
        assertEqualChar(i, writeBuffer[i]);
    }
    assertOK(externalStorage_openFile(testFileName, &file, FILE_MODE_WRITE));

    uint bytesWritten;
    assertOK(externalStorage_writeFile(file, /*startPosition=*/0, writeBuffer, bufferLength, &bytesWritten));
    assertEqualUInt(bufferLength, bytesWritten);
    assertOK(externalStorage_closeFile(file));
}

testCase("file read") {
    assertOK(externalStorage_openFile(testFileName, &file, FILE_MODE_READ));

    char *readBuffer = calloc(bufferLength, sizeof(char));
    for (int i = 0; i < bufferLength; i++) {
        readBuffer[i] = 0;
        assertEqualChar(0, readBuffer[i]);
    }

    uint bytesRead;
    assertOK(externalStorage_readFile(file, /*startPosition=*/0, readBuffer, bufferLength, &bytesRead));
    assertEqualUInt(bufferLength, bytesRead);
    for (int i = 0; i < bufferLength; i++) {
        assertEqualChar(writeBuffer[i], readBuffer[i]);
    }
    free(readBuffer);
    free(writeBuffer);
    assertOK(externalStorage_closeFile(file));
}

testCase("file move") {
    assertOK(externalStorage_moveFile(testFileName, newTestFileName));

    bool fileExists = true;
    assertOK(externalStorage_queryFileExists(testFileName, &fileExists));
    assertFalse(fileExists);

    assertOK(externalStorage_queryFileExists(newTestFileName, &fileExists));
    assertTrue(fileExists);
}

testCase("file delete") {
    assertOK(externalStorage_deleteFile(newTestFileName));

    bool fileExists = true;
    assertOK(externalStorage_queryFileExists(newTestFileName, &fileExists));
    assertFalse(fileExists);
}

testCase("query path type") {
    ensureDir(testDirName);
    ensureFile(testFileName);

    bool isDir, isFile;
    assertOK(externalStorage_queryPathType(testDirName, &isDir, &isFile));
    assertTrue(isDir);
    assertFalse(isFile);

    assertOK(externalStorage_queryPathType(testFileName, &isDir, &isFile));
    assertTrue(isFile);
    assertFalse(isDir);

    ensureNonExistentDir(testDirName);
    ensureNonExistentFile(testFileName);
}

testCase("read root dir") {
    clearDir("/");
    const int fileCount = 5;
    const int dirCount = 5;
    fillDir("/", fileCount, dirCount);

    size_t entryCount = SIZE_MAX;
    assertOK(externalStorage_readDir("/", NULL, &entryCount));
    assertEqualUInt(fileCount + dirCount, entryCount);

    char **dirEntries = createEmptyStringArray(entryCount);
    assertOK(externalStorage_readDir("/", dirEntries, &entryCount));
    for (int i = 0; i < entryCount; i++) {
        char *entry = dirEntries[i];
        assertNotNull(entry);
        free(dirEntries[i]);
        dirEntries[i] = NULL;
    }
    free(dirEntries);

    clearDir("/");
}

testCase("finish tests") {
    // delete all test files and dirs
    ensureNonExistentDir(testDirName);
    ensureNonExistentDir(newTestDirName);
    ensureNonExistentFile(testFileName);
    ensureNonExistentFile(newTestFileName);

    externalStorage_unmountSDCard(); // ok if failed
    assertOK(externalStorage_destroy());
}

endTestFile