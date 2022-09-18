#include "Assertions.h"
#include "TestUtils.h"
#include "InternalStorage.h"
#include <string.h>

beginTestFile("InternalStorage")

#define testFileName "testFile"
#define newTestFileName "newTestFile"

private inline void ensureNonExistentFile(const char *fileName) {
    bool fileExists;
    internalStorage_queryFileExists(fileName, &fileExists);
    if (fileExists) {
        assertOK(internalStorage_deleteFile(fileName));
        internalStorage_queryFileExists(fileName, &fileExists);
        assertFalse(fileExists);
    }
}

testCase("Storage init and destroy") {
    assertOK(internalStorage_init());
    assertOK(internalStorage_destroy());
}

run("init for following tests") {
    assertOK(internalStorage_init());
}

run("ensure file is non-existent for following tests") {
    ensureNonExistentFile(testFileName);
    ensureNonExistentFile(newTestFileName);
}

testCase("non existent file query exists") {
    bool fileExists = true;
    assertOK(internalStorage_queryFileExists(testFileName, &fileExists));
    assertFalse(fileExists);
}

testCase("non existent file query info") {
    FileInfo fileInfo;
    fileInfo.sizeBytes = UINT_MAX;
    assertEqualInt(STORAGE_ERROR_NOT_FOUND,
                   internalStorage_queryFileInfo(testFileName, &fileInfo));
    assertEqualUInt(UINT_MAX, fileInfo.sizeBytes); // no change if failed
}

testCase("non existent file open") {
    FILE *file;
    assertEqualInt(STORAGE_ERROR_NOT_FOUND,
                   internalStorage_openFile(testFileName, &file, FILE_MODE_WRITE));
}

testCase("non existent file close") {
    FILE *file = NULL;
    assertEqualInt(STORAGE_ERROR_INVALID_PARAMETER,
                   internalStorage_closeFile(file));
}

testCase("non existent file read") {
    FILE *file = NULL;
    const uint bufferSize = 512;
    char *buffer = calloc(bufferSize, sizeof(char));
    for (int i = 0; i < bufferSize; i++) {
        assertEqualChar(0, buffer[i]);
    }
    uint bytesRead = UINT_MAX;
    assertEqualInt(STORAGE_ERROR_INVALID_PARAMETER,
                   internalStorage_readFile(file, /*startPosition=*/0, buffer, bufferSize, &bytesRead));
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
    assertEqualInt(STORAGE_ERROR_INVALID_PARAMETER,
                   internalStorage_writeFile(file, /*startPosition=*/0, buffer, bufferSize, &bytesWritten));
    assertEqualUInt(UINT_MAX, bytesWritten); // no change if failed
    free(buffer);
}

testCase("non existent file move") {
    assertEqualInt(STORAGE_ERROR_NOT_FOUND,
                   internalStorage_moveFile(testFileName, newTestFileName));
}

testCase("non existent file delete") {
    assertEqualInt(STORAGE_ERROR_NOT_FOUND,
                   internalStorage_deleteFile(testFileName));
}

run("ensure file is non-existent for following tests") {
    ensureNonExistentFile(testFileName);
    ensureNonExistentFile(newTestFileName);
}

testCase("file create") {
    assertOK(internalStorage_createFile(testFileName));
}

testCase("file query exists") {
    bool fileExists = false;
    assertOK(internalStorage_queryFileExists(testFileName, &fileExists));
    assertTrue(fileExists);
}

testCase("file query info") {
    FileInfo fileInfo;
    fileInfo.sizeBytes = UINT_MAX;
    assertOK(internalStorage_queryFileInfo(testFileName, &fileInfo));
    assertNotEqualUInt(UINT_MAX, fileInfo.sizeBytes);
}

private FILE *file;

testCase("file open") {
    assertOK(internalStorage_openFile(testFileName, &file, FILE_MODE_WRITE));
    assertNotNull(file);
}

testCase("file close") {
    assertOK(internalStorage_closeFile(file));
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
    assertOK(internalStorage_openFile(testFileName, &file, FILE_MODE_WRITE));

    uint bytesWritten;
    assertOK(internalStorage_writeFile(file, /*startPosition=*/0, writeBuffer, bufferLength, &bytesWritten));
    assertEqualUInt(bufferLength, bytesWritten);
    assertOK(internalStorage_closeFile(file));
}

testCase("file read") {
    assertOK(internalStorage_openFile(testFileName, &file, FILE_MODE_READ));

    char *readBuffer = calloc(bufferLength, sizeof(char));
    for (int i = 0; i < bufferLength; i++) {
        readBuffer[i] = 0;
        assertEqualChar(0, readBuffer[i]);
    }

    uint bytesRead;
    assertOK(internalStorage_readFile(file, /*startPosition=*/0, readBuffer, bufferLength, &bytesRead));
    assertEqualUInt(bufferLength, bytesRead);
    for (int i = 0; i < bufferLength; i++) {
        assertEqualChar(writeBuffer[i], readBuffer[i]);
    }
    free(readBuffer);
    free(writeBuffer);
    assertOK(internalStorage_closeFile(file));
}

testCase("file move") {
    assertOK(internalStorage_moveFile(testFileName, newTestFileName));

    bool fileExists = true;
    assertOK(internalStorage_queryFileExists(testFileName, &fileExists));
    assertFalse(fileExists);

    assertOK(internalStorage_queryFileExists(newTestFileName, &fileExists));
    assertTrue(fileExists);
}

testCase("file delete") {
    assertOK(internalStorage_deleteFile(newTestFileName));

    bool fileExists = true;
    assertOK(internalStorage_queryFileExists(newTestFileName, &fileExists));
    assertFalse(fileExists);
}

testCase("finish tests") {
    ensureNonExistentFile(testFileName);
    ensureNonExistentFile(newTestFileName);

    assertOK(internalStorage_destroy());
}

endTestFile