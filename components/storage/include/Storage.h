#ifndef ESP32_REMOTECAMERA_STORAGE_H
#define ESP32_REMOTECAMERA_STORAGE_H

/** https://en.cppreference.com/w/c/io/fopen#File_access_flags */
typedef enum FileAccessMode {
    READ,
    WRITE,
    APPEND,
    READ_EXTENDED,
    WRITE_EXTENDED,
    APPEND_EXTENDED
} FileAccessMode;

#endif //ESP32_REMOTECAMERA_STORAGE_H
