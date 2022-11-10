#include "List.h"
#include "Utils.h"
#include <stdlib.h>

typedef struct ListData {
    /** List options set at creation time */
    ListOptions options;
    /** Actual current size */
    capacity_t size;
    /** The items, ie a list of pointers */
    ListItem **items;
} ListData;

#define callError(this, error) \
if ((this)->options.errorCallback) (this)->options.errorCallback(error)

/** Grow the list, only call when growth is confirmed to be needed as this does no checks */
private void list_grow(ListData *this) {
    const capacity_t newCapacity = this->options.capacity * this->options.growthFactor;
    this->items = realloc(this->items, newCapacity * sizeof(ListItem *));
    this->options.capacity = newCapacity;
}

/** Shift all elements from startIndex (inclusive) until last element forward by one */
private Error list_shiftRight(ListData *this, const index_t startIndex) {
    if (this->size == this->options.capacity) { // reached the limit, need to grow
        if (this->options.isGrowable) // grow only if enabled
            list_grow(this);
        else { // otherwise error and do nothing
            callError(this, LIST_ERROR_CAPACITY_EXCEEDED);
            return LIST_ERROR_CAPACITY_EXCEEDED;
        }
    }
    for (index_t i = this->size; i > startIndex; i--) { // must loop backwards
        this->items[i] = this->items[i - 1];
    }
    return ERROR_NONE;
}

/** Shift all elements from end until startIndex (inclusive) backward by one */
private void list_shiftLeft(ListData *this, const index_t startIndex) {
    for (index_t i = startIndex; i < this->size; i++) {
        this->items[i] = this->items[i + 1];
    }
}

public List *list_create() {
    return list_createWithOptions(NULL);
}

public List *list_createWithOptions(const ListOptions *listOptions) {
    ListData *this = new(ListData);
    ListOptions options = LIST_DEFAULT_OPTIONS;
    if (listOptions != NULL) {
        options = *listOptions;
    }
    this->options = options;
    this->size = 0;
    if (this->options.capacity <= 0) this->options.capacity = LIST_DEFAULT_INITIAL_CAPACITY;
    if (this->options.growthFactor <= 1) this->options.growthFactor = LIST_GROWTH_FACTOR;
    this->items = alloc(this->options.capacity * sizeof(ListItem *));
    return this;
}

public void list_destroy(List *list) {
    if (!list) return;
    ListData *this = (ListData *) list;
    delete(this->items);
    this->items = NULL;
    delete(this);
    this = NULL;
}

public capacity_t list_getSize(const List *list) {
    if (!list) return LIST_INVALID_INDEX_CAPACITY;
    ListData *this = (ListData *) list;
    return this->size;
}

public bool list_isEmpty(const List *list) {
    if (list) return list_getSize(list) == 0;
    else return true; // return true if list was null since it's technically empty
}

public ListItem *list_getItem(const List *list, const index_t index) {
    if (!list) return NULL;
    ListData *this = (ListData *) list;
    const index_t lastIndex = this->size - 1;
    if (index > lastIndex) {
        callError(this, ERROR_OUT_OF_BOUNDS);
        return NULL;
    }
    return this->items[index];
}

public Error list_setItem(const List *list, const index_t index, const ListItem *item) {
    if (!list) return ERROR_NULL_ARGUMENT;
    ListData *this = (ListData *) list;
    const index_t lastIndex = this->size - 1;
    if (index == this->size) { // index is the next writable index, equivalent to addItem()
        return list_addItem(list, item); // let addItem() handle possible growth
    } else if (index > lastIndex) { // index must be within current list bounds
        callError(this, ERROR_OUT_OF_BOUNDS);
        return ERROR_OUT_OF_BOUNDS;
    }
    this->items[index] = item;
    return ERROR_NONE;
}

public index_t list_indexOfItem(const List *list, const ListItem *item) {
    return list_indexOfItemFunction(list, item, list_equalityByAddress);
}

public index_t list_indexOfItemFunction(const List *list, const ListItem *item,
                                        const ListEqualityFunction equalityFunction) {
    if (!list) return LIST_INVALID_INDEX_CAPACITY;
    ListData *this = (ListData *) list;
    for (int i = 0; i < this->size; i++) {
        if (equalityFunction(this->items[i], item)) return i;
    }
    callError(this, ERROR_NOT_FOUND);
    return LIST_INVALID_INDEX_CAPACITY;
}

public Error list_addItem(const List *list, const ListItem *item) {
    if (!list) return ERROR_NULL_ARGUMENT;
    ListData *this = (ListData *) list;
    const index_t lastIndex = this->size - 1;
    if (this->size == this->options.capacity) { // reached the limit, need to grow
        if (this->options.isGrowable) // grow only if enabled
            list_grow(this);
        else { // otherwise error and do nothing
            callError(this, LIST_ERROR_CAPACITY_EXCEEDED);
            return LIST_ERROR_CAPACITY_EXCEEDED;
        }
    }
    this->items[lastIndex + 1] = item;
    this->size++;
    return ERROR_NONE;
}

public Error list_addItemIndexed(const List *list, const index_t index, const ListItem *item) {
    if (!list) return ERROR_NULL_ARGUMENT;
    ListData *this = (ListData *) list;

    if (index == this->size) { // index is the next writable index, equivalent to addItem()
        return list_addItem(list, item); // let addItem() handle possible growth
    } else if (index > this->size) {
        callError(this, ERROR_OUT_OF_BOUNDS);
        return ERROR_OUT_OF_BOUNDS;
    }
    Error err;
    if ((err = list_shiftRight(this, index)) == ERROR_NONE) {
        this->size++;
        return list_setItem(list, index, item);
    } else {
        return err;
    }
}

public Error list_removeItem(const List *list, const ListItem *item) {
    if (!list) return ERROR_NULL_ARGUMENT;
    ListData *this = (ListData *) list;
    const index_t itemIndex = list_indexOfItem(list, item);
    if (itemIndex == LIST_INVALID_INDEX_CAPACITY) { // item was not found
        callError(this, ERROR_NOT_FOUND);
        return ERROR_NOT_FOUND;
    } else {
        return list_removeItemIndexed(list, itemIndex);
    }
}

public Error list_removeItemIndexed(const List *list, const index_t index) {
    if (!list) return ERROR_NULL_ARGUMENT;
    ListData *this = (ListData *) list;
    const index_t lastIndex = this->size - 1;
    if (index > lastIndex) {
        callError(this, ERROR_OUT_OF_BOUNDS);
        return ERROR_OUT_OF_BOUNDS;
    } else if (index == lastIndex) {
        this->size--;
    } else {
        list_shiftLeft(this, index);
        this->size--;
    }
    return ERROR_NONE;
}

public Error list_clear(const List *list) {
    if (!list) return ERROR_NULL_ARGUMENT;
    ListData *this = (ListData *) list;
    this->size = 0;
    return ERROR_NONE;
}

public bool list_equalityByAddress(const ListItem *a, const ListItem *b) {
    return a == b;
}