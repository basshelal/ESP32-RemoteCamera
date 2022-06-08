#include "List.h"
#include "utils.h"
#include <stdlib.h>

typedef struct ListData {
    /** List options set at creation time */
    ListOptions options;
    /** Actual current size */
    capacity_t size;
    /** The items, ie a list of pointers */
    ListItem **items;
} ListData;

/** Call the error callback with the error if the callback exists */
private inline void list_error(const ListData *listData, const ListError error) {
    if (listData->options.errorCallback) listData->options.errorCallback(error);
}

/** Default equality function used by list_indexOfItem */
private bool list_defaultEquality(const ListItem *a, const ListItem *b) {
    return a == b;
}

/** Grow the list, only call when growth is confirmed to be needed as this does no checks */
private void list_grow(ListData *listData) {
    const capacity_t newCapacity = listData->options.capacity * listData->options.growthFactor;
    ListItem **newItems = calloc(newCapacity, sizeof(ListItem *));
    for (int i = 0; i < listData->options.capacity; i++) {
        newItems[i] = listData->items[i];
    }
    listData->options.capacity = newCapacity;
    free(listData->items);
    listData->items = newItems;
}

/** Shift all elements from startIndex (inclusive) until last element forward by one */
private void list_shiftRight(ListData *listData, const index_t startIndex) {
    if (listData->size == listData->options.capacity) { // reached the limit, need to grow
        if (listData->options.isGrowable) // grow only if enabled
            list_grow(listData);
        else { // otherwise error and do nothing
            list_error(listData, LIST_ERROR_CAPACITY_EXCEEDED);
            return;
        }
    }
    const index_t lastIndex = listData->size - 1;
    for (int i = lastIndex; i >= startIndex; i--) { // must loop backwards
        listData->items[i + 1] = listData->items[i];
    }
}

/** Shift all elements from end until endIndex (exclusive) backward by one */
private void list_shiftLeft(ListData *listData, const index_t endIndex) {
    for (int i = 0; i < endIndex; i++) {
        listData->items[i] = listData->items[i + 1];
    }
}

public ListOptions *list_defaultListOptions() {
    ListOptions *options = calloc(1, sizeof(ListOptions));
    ListOptions o = {
            .capacity = LIST_DEFAULT_INITIAL_CAPACITY,
            .isGrowable = true,
            .growthFactor = LIST_GROWTH_FACTOR,
            .errorCallback = NULL
    };
    *options = o;
    return options;
}

public List *list_create() {
    return list_createOptions(list_defaultListOptions());
}

public List *list_createOptions(const ListOptions *listOptions) {
    ListData *listData = calloc(1, sizeof(ListData));
    listData->options = listOptions ? *listOptions : *list_defaultListOptions();
    listData->size = 0;
    if (listData->options.capacity <= 0) listData->options.capacity = LIST_DEFAULT_INITIAL_CAPACITY;
    if (listData->options.growthFactor <= 1) listData->options.growthFactor = LIST_GROWTH_FACTOR;
    listData->items = calloc(listData->options.capacity, sizeof(ListItem *));
    return listData;
}

public void list_destroy(List *list) {
    if (!list) return;
    ListData *listData = (ListData *) list;
    free(listData->items);
    listData->items = NULL;
    free(listData);
    listData = NULL;
}

public capacity_t list_size(const List *list) {
    if (!list) return -1;
    ListData *listData = (ListData *) list;
    return listData->size;
}

public bool list_isEmpty(const List *list) {
    if (list) return list_size(list) == 0;
    else return true; // return true if list was null since it's technically empty
}

public ListItem *list_getItem(const List *list, const index_t index) {
    if (!list) return NULL;
    ListData *listData = (ListData *) list;
    const index_t lastIndex = listData->size - 1;
    if (index < 0) {
        list_error(listData, LIST_ERROR_NEGATIVE_INDEX);
        return NULL;
    }
    if (index > lastIndex) {
        list_error(listData, LIST_ERROR_INDEX_OUT_OF_BOUNDS);
        return NULL;
    }
    return listData->items[index];
}

public void list_setItem(const List *list, const index_t index, const ListItem *item) {
    if (!list) return;
    ListData *listData = (ListData *) list;
    const index_t lastIndex = listData->size - 1;
    if (index < 0) {
        list_error(listData, LIST_ERROR_NEGATIVE_INDEX);
        return;
    }
    if (index > lastIndex) {
        list_error(listData, LIST_ERROR_INDEX_OUT_OF_BOUNDS);
        return;
    }
    listData->items[index] = item;
}

public index_t list_indexOfItem(const List *list, const ListItem *item) {
    return list_indexOfItemFunction(list, item, list_defaultEquality);
}

public index_t list_indexOfItemFunction(const List *list, const ListItem *item,
                                        const ListEqualityFunction equalityFunction) {
    if (!list) return -1;
    ListData *listData = (ListData *) list;
    for (int i = 0; i < listData->size; i++) {
        if (equalityFunction(listData->items[i], item)) return i;
    }
    list_error(listData, LIST_ERROR_ITEM_NOT_FOUND);
    return -1;
}

public void list_addItem(const List *list, const ListItem *item) {
    if (!list) return;
    ListData *listData = (ListData *) list;
    const index_t lastIndex = listData->size - 1;
    if (listData->size == listData->options.capacity) { // reached the limit, need to grow
        if (listData->options.isGrowable) // grow only if enabled
            list_grow(listData);
        else { // otherwise error and do nothing
            list_error(listData, LIST_ERROR_CAPACITY_EXCEEDED);
            return;
        }
    }
    list_setItem(list, lastIndex, item);
    listData->size++;
}

public void list_addItemIndexed(const List *list, const index_t index, const ListItem *item) {
    if (!list) return;
    ListData *listData = (ListData *) list;
    const index_t lastIndex = listData->size - 1;
    if (index < 0) {
        list_error(listData, LIST_ERROR_NEGATIVE_INDEX);
        return;
    }
    if (index > lastIndex) {
        list_error(listData, LIST_ERROR_INDEX_OUT_OF_BOUNDS);
        return;
    }
    if (index == lastIndex) {
        list_addItem(list, item);
        return;
    } else {
        list_shiftRight(listData, index);
        listData->size++;
        list_setItem(list, lastIndex, item);
    }
}

public void list_removeItem(const List *list, const ListItem *item) {
    if (!list) return;
    ListData *listData = (ListData *) list;
    const index_t itemIndex = list_indexOfItem(list, item);
    if (itemIndex < 0) { // item was not found
        list_error(listData, LIST_ERROR_ITEM_NOT_FOUND);
        return;
    } else {
        list_removeItemIndexed(list, itemIndex);
    }
}

public void list_removeItemIndexed(const List *list, const index_t index) {
    if (!list) return;
    ListData *listData = (ListData *) list;
    const index_t lastIndex = listData->size - 1;
    if (index < 0) {
        list_error(listData, LIST_ERROR_NEGATIVE_INDEX);
        return;
    }
    if (index > lastIndex) {
        list_error(listData, LIST_ERROR_INDEX_OUT_OF_BOUNDS);
        return;
    }
    if (index == lastIndex) {
        listData->size--;
        return;
    } else {
        list_shiftLeft(listData, index);
        listData->size--;
    }
}

