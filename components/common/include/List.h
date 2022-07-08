#ifndef ESP32_REMOTECAMERA_LIST_H
#define ESP32_REMOTECAMERA_LIST_H

#include <stdbool.h>
#include <stddef.h>

/**
 * A simple list data structure that can grow when necessary.
 * The lifetime of the ListItems will be the same as the lifetime of when they were created because
 * the List only stores references to the items (to avoid unnecessary memory copies and complexity).
 * It is your responsibility to ensure that the items retrieved from a list are safe to use and have
 * not run out of scope or been freed. A general rule of thumb is to avoid any stack data (function local
 * variables) unless the list itself is also on the stack. Otherwise, you should free the items only
 * after you have destroyed the list or at the very least, stopped using it.
 */

/** Opaque pointer for a List */
typedef void List;

/** Opaque pointer for a ListItem, safe to cast to any pointer */
typedef void ListItem;

typedef int index_t;

typedef int capacity_t;

typedef enum ListError {
    /** When capacity exceeded and must grow but isGrowable is false */
    LIST_ERROR_CAPACITY_EXCEEDED,
    /** When a requested item was not found */
    LIST_ERROR_ITEM_NOT_FOUND,
    /** Passed in index was negative */
    LIST_ERROR_NEGATIVE_INDEX,
    /** Passed in index was out of bounds (but positive) */
    LIST_ERROR_INDEX_OUT_OF_BOUNDS,
} ListError;

typedef void (*ListErrorCallback)(const ListError error);
typedef bool (*ListEqualityFunction)(const ListItem *a, const ListItem *b);

typedef struct ListOptions {
    /** Max possible size before needing to grow, must be greater than 0 otherwise default will be used */
    capacity_t capacity;
    /** true if list can grow, otherwise list will never grow when needed */
    bool isGrowable;
    /** Factor to use when needing to grow, must be greater than 1 otherwise default will be used */
    capacity_t growthFactor;
    /** Function to be called when an error occurs */
    ListErrorCallback errorCallback;
} ListOptions;

/** Factor to use when needing to grow */
#define LIST_GROWTH_FACTOR 2
/** Initial capacity when none was given */
#define LIST_DEFAULT_INITIAL_CAPACITY 10

/** Create a new ListOptions with default values, this performs a call to malloc */
extern ListOptions *list_defaultListOptions();

/** Create and return a list with default options from list_defaultListOptions() */
extern List *list_create();

/** Create and return a new list with the list options, if options was NULL will use list_defaultListOptions() */
extern List *list_createOptions(const ListOptions *listOptions);

/** Destroy the list and free used resources, list is unsafe to use after this call, items will still be safe to use */
extern void list_destroy(List *list);

/** Get the current size of the list, -1 if the list is NULL */
extern capacity_t list_size(const List *list);

/** true if the list is empty ie size == 0 (or NULL), false otherwise */
extern bool list_isEmpty(const List *list);

/** Get the item at index or NULL if the index is out of bounds */
extern ListItem *list_getItem(const List *list, const index_t index);

/** Set the item at the index, the list will not be modified in any other way,
 * does nothing if list is NULL or index is out of bounds */
extern void list_setItem(const List *list, const index_t index, const ListItem *item);

/** Get the index of the first instance of item or -1 if it was not found or list was NULL
 * item can be NULL, in which case will find the first instance of NULL in the list,
 * the equality function used is based on pointer address (identity equals) */
extern index_t list_indexOfItem(const List *list, const ListItem *item);

/** Same as list_indexOfItem but using the passed in equality function for
 * equality checking instead of using pointer addresses */
extern index_t list_indexOfItemFunction(const List *list, const ListItem *item,
                                        const ListEqualityFunction equalityFunction);

/** Add item to the back/end of the list, growing the list if necessary and enabled
 * if growing is necessary but not enabled then no modifications are made */
extern void list_addItem(const List *list, const ListItem *item);

/** Add item to the index, pushing forward items with a higher index and growing if necessary */
extern void list_addItemIndexed(const List *list, const index_t index, const ListItem *item);

/** Removes the item if it was found in the list, pushing back items with a higher index if necessary */
extern void list_removeItem(const List *list, const ListItem *item);

/** Removes the item at the index, pushing back items with a higher index if necessary  */
extern void list_removeItemIndexed(const List *list, const index_t index);

#endif //ESP32_REMOTECAMERA_LIST_H
