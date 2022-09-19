#include <sys/dirent.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include "StorageCommon.h"
#include "StorageInfo.h"
#include "ExternalStorage.h"
#include "Logger.h"
#include "Utils.h"

#define MAX_PATH_LENGTH EXTERNAL_STORAGE_MAX_PATH_LENGTH

private Error storage_deleteDirAndContents(const char *dirPath) {
    List *stack = list_create();
    List *dirsToDelete = list_create();
    char pathBuffer[MAX_PATH_LENGTH];
    struct dirent *entry;

    // This algorithm is essentially an iterative Depth First Search using a stack (List made to behave like a stack)
    //  we delete files as we find them, and add dirs in the order which we encounter them which will be from
    //  highest to lowest in the chain, meaning that list reversed will always be safe to delete because the dirs
    //  will be empty
    list_addItem(stack, dirPath); // begin with root dir
    while (!list_isEmpty(stack)) {
        const index_t stackLastIndex = list_getSize(stack) - 1;

        // all "processing" is done on the popped dir which is like the "current" dir
        const char *poppedDir = list_getItem(stack, stackLastIndex);
        list_removeItemIndexed(stack, stackLastIndex);
        list_addItem(dirsToDelete, poppedDir); // move from stack to dirsToDelete list

        const DIR *dir = opendir(poppedDir);
        if (dir == NULL) continue;

        while ((entry = readdir(dir))) {
            sprintf(pathBuffer, "%s/%s", poppedDir, entry->d_name);
            if (entry->d_type == DT_REG) { // file
                remove(pathBuffer); // delete files as we find them
            } else if (entry->d_type == DT_DIR) { // dir
                char *pathCopy = alloc(strlen(pathBuffer) + 1);
                strcpy(pathCopy, pathBuffer);
                list_addItem(stack, pathCopy); // add to stack to be processed on next loop
            }
        }
        closedir(dir);
    }
    // Reverse loop on dirsToDelete ensures that rmdir calls are always done on empty dirs
    for (int i = (int) list_getSize(dirsToDelete) - 1; i >= 0; i--) {
        const char *dirToDelete = list_getItem(dirsToDelete, i);
        if (dirToDelete == NULL) continue;
        if ((rmdir(dirToDelete) != 0)) {
            int err = errno;
            throwLibCError(rmdir(), err);
        }
        if (dirToDelete != dirPath) { // all but root were allocated, so they need to be freed
            free(dirToDelete);
        }
    }
    list_destroy(stack);
    list_destroy(dirsToDelete);
    return ERROR_NONE;
}

/*============================= Directories =================================*/

public Error storage_queryDirExists(const char *dirPath, bool *dirExists) {
    DIR *dir = opendir(dirPath);
    if (dir == NULL) {
        int err = errno;
        if (err == ENOENT) {
            *dirExists = false;
        } else {
            throwLibCError(opendir(), err);
        }
    } else {
        *dirExists = true;
        closedir(dir);
    }
    return ERROR_NONE;
}

public Error storage_queryDirInfo(const char *dirPath, DirInfo *dirInfo) {
    struct stat statResult;
    if (stat(dirPath, &statResult) != 0) {
        int err = errno;
        if (err == ENOENT || err == ENOTDIR) {
            throw(ERROR_NOT_FOUND, "dir not found: %s", dirPath);
        } else if (err != 0) {
            throwLibCError(stat(), err);
        } else {
            // TODO: 04-Sep-2022 @basshelal: Implement or reconsider API
        }
    }
    return ERROR_NONE;
}

public Error storage_createDir(const char *dirPath) {
    if (mkdir(dirPath, ACCESSPERMS) != 0) {
        int err = errno;
        throwLibCError(mkdir(), err);
    }

    return ERROR_NONE;
}

public Error storage_readDir(const char *dirPath, char **dirEntries, size_t *entryCount) {
    bool fillEntries = true;
    if (dirEntries == NULL) { // user only wants count and not to fill array
        fillEntries = false;
    }

    DIR *dir = opendir(dirPath);
    if (dir == NULL) {
        int err = errno;
        if (err == ENOENT) {
            *entryCount = 0;
            throw(ERROR_NOT_FOUND, "dir not found: %s", dirPath);
        } else {
            throwLibCError(opendir(), err);
        }
    }
    size_t entries = 0;
    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (fillEntries) {
            dirEntries[entries] = alloc(sizeof(char) * strlen(entry->d_name));
            strcpy(dirEntries[entries], entry->d_name);
        }
        entries++;
    }
    *entryCount = entries;

    return ERROR_NONE;
}

public Error storage_moveDir(const char *dirPath, const char *newDirPath) {
    if (rename(dirPath, newDirPath) != 0) {
        int err = errno;
        if (err == ENOENT) {
            throw(ERROR_NOT_FOUND, "dir(s) not found: %s -> %s", dirPath, newDirPath);
        } else {
            throwLibCError(rename(), err);
        }
    }
    return ERROR_NONE;
}

public Error storage_deleteDir(const char *dirPath) {
    bool dirExists;
    storage_queryDirExists(dirPath, &dirExists);
    if (!dirExists) {
        throw(ERROR_NOT_FOUND, "dir not found: %s", dirPath);
    }
    throwIfError(storage_deleteDirAndContents(dirPath), "could not delete dir and/or contents");
    return ERROR_NONE;
}

/*============================= Files =======================================*/

public Error storage_queryFileExists(const char *filePath, bool *fileExists) {
    return storage_queryFileExistsStat(filePath, fileExists);
}

public Error storage_queryFileExistsAccess(const char *filePath, bool *fileExists) {
    if (access(filePath, F_OK) == 0) {
        *fileExists = true;
    } else {
        int err = errno;
        if (errno == ENOENT) {
            *fileExists = false;
        } else {
            throwLibCError(access(), err);
        }
    }
    return ERROR_NONE;
}

public Error storage_queryFileExistsStat(const char *filePath, bool *fileExists) {
    struct stat statResult;
    if (stat(filePath, &statResult) == 0) {
        *fileExists = true;
    } else {
        int err = errno;
        if (errno == ENOENT) {
            *fileExists = false;
        } else {
            throwLibCError(stat(), err);
        }
    }
    return ERROR_NONE;
}

public Error storage_queryFileInfo(const char *filePath, FileInfo *fileInfo) {
    struct stat statResult;
    if ((stat(filePath, &statResult)) != 0) {
        int err = errno;
        if (errno == ENOENT) {
            throw(ERROR_NOT_FOUND, "file not found: %s", filePath);
        } else {
            throwLibCError(stat(), err);
        }
    }
    fileInfo->sizeBytes = (uint32_t) statResult.st_size;

    return ERROR_NONE;
}

public Error storage_createFile(const char *filePath) {
    bool fileExists;
    storage_queryFileExists(filePath, &fileExists);
    if (!fileExists) {
        FILE *file = fopen(filePath, "w");
        if (file != NULL) {
            fclose(file);
        } else {
            int err = errno;
            throwLibCError(fopen(), err);
        }
    }
    return ERROR_NONE;
}

public Error storage_openFile(const char *filePath, FILE **fileIn, const FileMode fileMode) {
    bool fileExists;
    throwIfError(storage_queryFileExists(filePath, &fileExists),
                 "could not check if file exists before opening: %s", filePath);
    if (!fileExists) {
        throw(ERROR_NOT_FOUND, "file not found: %s", filePath);
    }

    FILE *file = fopen(filePath, fileModeToString(fileMode));
    if (file == NULL) {
        int err = errno;
        if (err == ENOENT) {
            throw(ERROR_NOT_FOUND, "file not found: %s", filePath);
        } else {
            throwLibCError(fopen(), err);
        }
    } else {
        *fileIn = file;
    }

    return ERROR_NONE;
}

public Error storage_closeFile(const FILE *fileIn) {
    if ((fclose(fileIn) != 0)) {
        int err = errno;
        throwLibCError(fclose(), err);
    }
    return ERROR_NONE;
}

public Error storage_readFile(const FILE *file, size_t startPosition,
                              void *bufferIn, const uint bufferLength, uint *bytesRead) {
    int err;
    if ((err = fsetpos(file, (fpos_t *) &startPosition))) {
        throwLibCError(fsetpos(), err);
    }
    *bytesRead = fread(bufferIn, sizeof(char), bufferLength, file);
    if ((err = ferror(file))) {
        throwLibCError(fread(), err);
    }

    return ERROR_NONE;
}

public Error storage_writeFile(const FILE *file, size_t startPosition,
                               const void *buffer, const uint bufferLength, uint *bytesWritten) {
    int err;
    if ((err = fsetpos(file, (fpos_t *) &startPosition))) {
        throwLibCError(fsetpos(), err);
    }
    *bytesWritten = fwrite(buffer, sizeof(char), bufferLength, file);
    if ((err = ferror(file))) {
        throwLibCError(fwrite(), err);
    }

    return ERROR_NONE;
}

public Error storage_moveFile(const char *filePath, const char *newFilePath) {
    if ((rename(filePath, newFilePath) != 0)) {
        int err = errno;
        if (err == ENOENT) {
            throw(ERROR_NOT_FOUND, "file(s) not found: %s -> %s", filePath, newFilePath);
        } else {
            throwLibCError(rename(), err);
        }
    }

    return ERROR_NONE;
}

public Error storage_deleteFile(const char *filePath) {
    if (remove(filePath) != 0) {
        int err = errno;
        if (err == ENOENT) {
            throw(ERROR_NOT_FOUND, "file not found: %s", filePath);
        } else {
            throwLibCError(remove(), err);
        }
    }
    return ERROR_NONE;
}