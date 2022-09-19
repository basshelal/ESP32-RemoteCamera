#ifndef ESP32_REMOTECAMERA_ERROR_H
#define ESP32_REMOTECAMERA_ERROR_H

#include "Constants.h"

typedef enum {
    /* General/Common Errors =================================================================== */
    ERROR_NONE = 0,
    ERROR_UNKNOWN = -1,
    ERROR_ILLEGAL_ARGUMENT,
    ERROR_ILLEGAL_STATE,
    ERROR_NULL_ARGUMENT,
    ERROR_NOT_FOUND,
    ERROR_NOT_INITIALIZED,
    ERROR_OUT_OF_BOUNDS,
    ERROR_LIBRARY_FAILURE,
    /* List Errors ============================================================================= */
    /** When capacity exceeded and must grow but isGrowable is false */
    LIST_ERROR_CAPACITY_EXCEEDED,
} Error;

inline const char *errorToString(Error error) {
#if CONFIG_USE_ERROR_TO_STRING
    return "";
#else
    return "";
#endif // CONFIG_USE_ERROR_TO_STRING
}

#endif //ESP32_REMOTECAMERA_ERROR_H
