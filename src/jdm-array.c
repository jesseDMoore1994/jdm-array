#include "jdm-array.h"
#include <jdm-log.h>
#include <stdlib.h>
#include <string.h>

#define JDM_ARRAY_INDEX(a, i) a->element_size * (i)
#define JDM_ARRAY_BUF_ADDR(a, i) &a->mem[a->element_size * (i)]

struct jdm_array_t {
    jdm_alloc_t* alloc;
    sz size;
    sz capacity;
    sz element_size;
    f32 exp_factor;
    u8* mem;
};

jdm_array_t* jdm_array_create(const jdm_array_config_t cfg) {
    jdm_array_t* a = jdm_alloc(cfg.alloc, sizeof(jdm_array_t));
    if (a == JDM_NULL) {
        return JDM_NULL;
    }

    a->alloc = cfg.alloc;
    a->size = 0;
    a->capacity = cfg.capacity;
    a->element_size = cfg.element_size != 0 ? cfg.element_size : sizeof(pointer);
    a->exp_factor = cfg.exp_factor;
    a->mem = jdm_alloc(cfg.alloc, a->capacity * a->element_size);
    if (a->mem == JDM_NULL) {
        jdm_free(cfg.alloc, a);
        return JDM_NULL;
    }

    return a;
}

void jdm_array_destroy(jdm_array_t* arr) {
    jdm_alloc_t* alloc = arr->alloc;
    jdm_free(alloc, arr->mem);
    jdm_free(alloc, arr);
}

void jdm_array_print(jdm_array_t* arr, sz elem_size, jdm_array_print_elem_func_t print_elem) {
    JDM_LOG_INFO("Array size: %zu, capacity: %zu", arr->size, arr->capacity);
    for (sz i = 0; i < arr->size; i++) {
        print_elem(JDM_ARRAY_BUF_ADDR(arr, i), i);
    }
    
    // raw dump of memory in hex (print an element on each line)
    JDM_LOG_DEBUG("Raw memory dump:");
    for (sz i = 0; i < arr->size; i++) {
        u8* elem = JDM_ARRAY_BUF_ADDR(arr, i);
        JDM_LOG_DEBUG("Element[%zu]:", i);
        for (sz j = 0; j < elem_size; j++) {
            JDM_LOG_DEBUG("%02X ", elem[j]);
        }
        JDM_LOG_DEBUG("----");
    }
}

#define JDM_ARRAY_MAX_ELEMENTS ((sz) - 2)
static jdm_array_status jdm_array_expand_capacity(jdm_array_t* arr) {
    if (arr->capacity == JDM_ARRAY_MAX_ELEMENTS) {
        JDM_LOG_ERROR("Array has reached maximum capacity");
        return JDM_ARRAY_ERR_MAX_CAP;
    }

    sz new_cap = (sz) (arr->capacity * arr->exp_factor);
    if (new_cap <= arr->capacity) {
        arr->capacity = JDM_ARRAY_MAX_ELEMENTS;
    } else {
        arr->capacity = new_cap;
    }

    u8* new_mem = jdm_alloc(arr->alloc, (arr->capacity * arr->element_size));
    if (!new_mem) {
        JDM_LOG_ERROR("Failed to allocate memory for array expansion");
        return JDM_ARRAY_ERR_ALLOC;
    }
    JDM_COPY_MEMORY(new_mem, arr->mem, arr->size * arr->element_size);

    jdm_free(arr->alloc, arr->mem);
    arr->mem = new_mem;
    return JDM_ARRAY_OK;

}

jdm_array_status jdm_array_add(jdm_array_t* arr, u8* elem) {
    if (arr->size >= arr->capacity) {
        jdm_array_status s = jdm_array_expand_capacity(arr);
        if (s != JDM_ARRAY_OK) {
            JDM_LOG_ERROR("Failed to expand array capacity");
            return s;
        }
    }

    JDM_COPY_MEMORY(
        JDM_ARRAY_BUF_ADDR(arr, arr->size),
        elem,
        arr->element_size
    );

    arr->size++;
    return JDM_ARRAY_OK;
}

jdm_array_status jdm_array_add_at(jdm_array_t* arr, u8* elem, sz index) {
    if (index > arr->size) {
        JDM_LOG_ERROR("Index out of bounds for array insertion");
        return JDM_ARRAY_ERR_INVALID_INDEX;
    }

    if (arr->size >= arr->capacity) {
        jdm_array_status s = jdm_array_expand_capacity(arr);
        if (s != JDM_ARRAY_OK) {
            JDM_LOG_ERROR("Failed to expand array capacity");
            return s;
        }
    }

    // Shift elements to the right
    JDM_COPY_MEMORY(
        JDM_ARRAY_BUF_ADDR(arr, index + 1),
        JDM_ARRAY_BUF_ADDR(arr, index),
        (arr->size - index) * arr->element_size
    );

    // Copy new element into position
    JDM_COPY_MEMORY(
        JDM_ARRAY_BUF_ADDR(arr, index),
        elem,
        arr->element_size
    );

    arr->size++;
    return JDM_ARRAY_OK;
}

jdm_array_status jdm_array_replace_at(jdm_array_t* arr, u8* elem, sz index) {
    if (index >= arr->size) {
        JDM_LOG_ERROR("Index out of bounds for array replacement");
        return JDM_ARRAY_ERR_INVALID_INDEX;
    }

    JDM_COPY_MEMORY(
        JDM_ARRAY_BUF_ADDR(arr, index),
        elem,
        arr->element_size
    );

    return JDM_ARRAY_OK;
}

jdm_array_status jdm_array_swap_at(jdm_array_t* arr, sz index1, sz index2) {
    if (index1 >= arr->size || index2 >= arr->size) {
        JDM_LOG_ERROR("Index out of bounds for array swap");
        return JDM_ARRAY_ERR_INVALID_INDEX;
    }

    u8* temp = jdm_alloc(arr->alloc, arr->element_size);
    if (!temp) {
        JDM_LOG_ERROR("Failed to allocate memory for array swap");
        return JDM_ARRAY_ERR_ALLOC;
    }

    JDM_COPY_MEMORY(temp, JDM_ARRAY_BUF_ADDR(arr, index1), arr->element_size);
    JDM_COPY_MEMORY(JDM_ARRAY_BUF_ADDR(arr, index1), JDM_ARRAY_BUF_ADDR(arr, index2), arr->element_size);
    JDM_COPY_MEMORY(JDM_ARRAY_BUF_ADDR(arr, index2), temp, arr->element_size);

    jdm_free(arr->alloc, temp);
    return JDM_ARRAY_OK;
}

jdm_array_status jdm_array_remove(jdm_array_t* arr, u8* elem) {
    sz index;
    jdm_array_status s = jdm_array_index_of(arr, elem, &index);
    if (s != JDM_ARRAY_OK) {
        JDM_LOG_ERROR("Element not found in array for removal");
        return s;
    }
    return jdm_array_remove_at(arr, index, JDM_NULL);
}

jdm_array_status jdm_array_remove_at(jdm_array_t* arr, sz index, u8* out_elem) {
    if (index >= arr->size) {
        JDM_LOG_ERROR("Index out of bounds for array removal");
        return JDM_ARRAY_ERR_INVALID_INDEX;
    }

    if (out_elem) {
        JDM_COPY_MEMORY(out_elem, JDM_ARRAY_BUF_ADDR(arr, index), arr->element_size);
    }

    // Shift elements to the left
    JDM_COPY_MEMORY(
        JDM_ARRAY_BUF_ADDR(arr, index),
        JDM_ARRAY_BUF_ADDR(arr, index + 1),
        (arr->size - index - 1) * arr->element_size
    );

    arr->size--;
    return JDM_ARRAY_OK;
}

jdm_array_status jdm_array_remove_last(jdm_array_t* arr, u8* out_elem) {
    if (arr->size == 0) {
        JDM_LOG_ERROR("Array is empty, cannot remove last element");
        return JDM_ARRAY_ERR_INVALID_INDEX;
    }

    if (out_elem) {
        JDM_COPY_MEMORY(out_elem, JDM_ARRAY_BUF_ADDR(arr, arr->size - 1), arr->element_size);
    }

    arr->size--;
    return JDM_ARRAY_OK;
}

jdm_array_status jdm_array_remove_all(jdm_array_t* arr) {
    arr->size = 0;
    return JDM_ARRAY_OK;
}

jdm_array_status jdm_array_get_at(jdm_array_t* arr, u8** elem, sz index) {
    if (index >= arr->size) {
        JDM_LOG_ERROR("Index out of bounds for array retrieval");
        return JDM_ARRAY_ERR_INVALID_INDEX;
    }

    JDM_COPY_MEMORY(*elem, JDM_ARRAY_BUF_ADDR(arr, index), arr->element_size);
    return JDM_ARRAY_OK;
}

jdm_array_status jdm_array_get_last(jdm_array_t* arr, u8** elem) {
    if (arr->size == 0) {
        JDM_LOG_ERROR("Array is empty, cannot retrieve last element");
        return JDM_ARRAY_ERR_INVALID_INDEX;
    }

    return jdm_array_get_at(arr, elem, arr->size - 1);
}

jdm_array_status jdm_array_peek(jdm_array_t* arr, u8** elem, sz index) {
    if (index >= arr->size) {
        JDM_LOG_ERROR("Index out of bounds for array peek");
        return JDM_ARRAY_ERR_INVALID_INDEX;
    }

    *elem = JDM_ARRAY_BUF_ADDR(arr, index);
    return JDM_ARRAY_OK;
}

jdm_array_status jdm_array_index_of(jdm_array_t* arr, u8* elem, sz* out_index) {
    for (sz i = 0; i < arr->size; i++) {
        if (memcmp(JDM_ARRAY_BUF_ADDR(arr, i), elem, arr->element_size) == 0) {
            *out_index = i;
            return JDM_ARRAY_OK;
        }
    }
    JDM_LOG_ERROR("Element not found in array");
    return JDM_ARRAY_ERR_NOT_FOUND;
}

jdm_array_status jdm_array_subarray(jdm_array_t* arr, jdm_array_t** out_subarray, sz start_index, sz end_index) {
    if (start_index >= arr->size || end_index > arr->size || start_index >= end_index) {
        JDM_LOG_ERROR("Invalid indices for subarray creation");
        return JDM_ARRAY_ERR_INVALID_INDEX;
    }

    sz subarray_size = end_index - start_index;
    jdm_array_config_t cfg = {
        .alloc = arr->alloc,
        .capacity = subarray_size,
        .element_size = arr->element_size,
        .exp_factor = arr->exp_factor
    };

    jdm_array_t* subarr = jdm_array_create(cfg);
    if (!subarr) {
        JDM_LOG_ERROR("Failed to create subarray");
        return JDM_ARRAY_ERR_ALLOC;
    }

    for (sz i = 0; i < subarray_size; i++) {
        jdm_array_add(subarr, JDM_ARRAY_BUF_ADDR(arr, start_index + i));
    }

    *out_subarray = subarr;
    return JDM_ARRAY_OK;
}

jdm_array_status jdm_array_copy(jdm_array_t* arr, jdm_array_t** out_copy) {
    jdm_array_config_t cfg = {
        .alloc = arr->alloc,
        .capacity = arr->capacity,
        .element_size = arr->element_size,
        .exp_factor = arr->exp_factor
    };

    jdm_array_t* copy = jdm_array_create(cfg);
    if (!copy) {
        JDM_LOG_ERROR("Failed to create array copy");
        return JDM_ARRAY_ERR_ALLOC;
    }

    JDM_COPY_MEMORY(copy->mem, arr->mem, arr->size * arr->element_size);
    copy->size = arr->size;

    *out_copy = copy;
    return JDM_ARRAY_OK;
}

jdm_array_status jdm_array_reverse(jdm_array_t* arr) {
    for (sz i = 0; i < arr->size / 2; i++) {
        jdm_array_swap_at(arr, i, arr->size - 1 - i);
    }
    return JDM_ARRAY_OK;
}

sz jdm_array_count_of(jdm_array_t* arr, u8* elem) {
    sz count = 0;
    for (sz i = 0; i < arr->size; i++) {
        if (memcmp(JDM_ARRAY_BUF_ADDR(arr, i), elem, arr->element_size) == 0) {
            count++;
        }
    }
    return count;
}

jdm_array_status jdm_array_trim_capacity(jdm_array_t* arr) {
    if (arr->size == arr->capacity) {
        return JDM_ARRAY_OK; // No need to trim
    }

    u8* new_mem = jdm_alloc(arr->alloc, arr->size * arr->element_size);
    if (!new_mem) {
        JDM_LOG_ERROR("Failed to allocate memory for array trimming");
        return JDM_ARRAY_ERR_ALLOC;
    }

    JDM_COPY_MEMORY(new_mem, arr->mem, arr->size * arr->element_size);
    jdm_free(arr->alloc, arr->mem);
    arr->mem = new_mem;
    arr->capacity = arr->size;

    return JDM_ARRAY_OK;
}

jdm_array_status jdm_array_sort(jdm_array_t* arr, jdm_array_compare_func_t compare) {
    if (arr->size <= 1) {
        return JDM_ARRAY_OK; // Already sorted
    }

    // Use qsort from the C standard library
    qsort(arr->mem, arr->size, arr->element_size, compare);
    return JDM_ARRAY_OK;
}

jdm_array_status jdm_array_map(jdm_array_t* arr, jdm_array_map_func_t map_func) {
    for (sz i = 0; i < arr->size; i++) {
        map_func(JDM_ARRAY_BUF_ADDR(arr, i));
    }
    return JDM_ARRAY_OK;
}

jdm_array_status jdm_array_reduce(jdm_array_t* arr, jdm_array_reduce_func_t reduce_func, pointer out) {
    for (sz i = 0; i < arr->size; i++) {
        reduce_func(JDM_ARRAY_BUF_ADDR(arr, i), out);
    }
    return JDM_ARRAY_OK;
}

jdm_array_status jdm_array_filter_mutable(jdm_array_t* arr, jdm_array_predicate_func_t predicate_func) {
    sz new_size = 0;
    for (sz i = 0; i < arr->size; i++) {
        if (predicate_func(JDM_ARRAY_BUF_ADDR(arr, i))) {
            if (new_size != i) {
                JDM_COPY_MEMORY(JDM_ARRAY_BUF_ADDR(arr, new_size), JDM_ARRAY_BUF_ADDR(arr, i), arr->element_size);
            }
            new_size++;
        }
    }
    arr->size = new_size;
    return JDM_ARRAY_OK;
}

jdm_array_status jdm_array_filter(jdm_array_t* arr, jdm_array_t** out_filtered, jdm_array_predicate_func_t predicate_func) {
    jdm_array_config_t cfg = {
        .alloc = arr->alloc,
        .capacity = arr->capacity,
        .element_size = arr->element_size,
        .exp_factor = arr->exp_factor
    };

    jdm_array_t* filtered = jdm_array_create(cfg);
    if (!filtered) {
        JDM_LOG_ERROR("Failed to create filtered array");
        return JDM_ARRAY_ERR_ALLOC;
    }

    for (sz i = 0; i < arr->size; i++) {
        if (predicate_func(JDM_ARRAY_BUF_ADDR(arr, i))) {
            jdm_array_add(filtered, JDM_ARRAY_BUF_ADDR(arr, i));
        }
    }

    *out_filtered = filtered;
    return JDM_ARRAY_OK;
}

sz jdm_array_element_size(jdm_array_t* arr) {
    return arr->element_size;
}

sz jdm_array_size(jdm_array_t* arr) {
    return arr->size;
}

sz jdm_array_capacity(jdm_array_t* arr) {
    return arr->capacity;
}

jdm_array_iterator_t jdm_array_iterator_create(jdm_array_t* arr) {
    return (jdm_array_iterator_t) {
        .arr = arr,
        .current_index = 0,
        .last_removed = JDM_FALSE
    };
}

jdm_array_status jdm_array_iterator_next(jdm_array_iterator_t* it, u8** elem) {
    if (it->current_index >= it->arr->size) {
        return JDM_ARRAY_ERR_INVALID_INDEX;
    }
    *elem = JDM_ARRAY_BUF_ADDR(it->arr, it->current_index);
    it->current_index++;
    it->last_removed = JDM_FALSE;
    return JDM_ARRAY_OK;
}

jdm_array_status jdm_array_iterator_remove(jdm_array_iterator_t* it, u8* out_elem) {
    if (it->last_removed) {
        JDM_LOG_ERROR("Cannot remove the same element twice in a row");
        return JDM_ARRAY_ERR_INVALID_INDEX;
    }
    if (it->current_index == 0) {
        JDM_LOG_ERROR("No element to remove, iterator is at the beginning");
        return JDM_ARRAY_ERR_INVALID_INDEX;
    }
    sz index_to_remove = it->current_index - 1;
    jdm_array_status s = jdm_array_remove_at(it->arr, index_to_remove, out_elem);
    if (s != JDM_ARRAY_OK) {
        return s;
    }
    it->current_index--; // Adjust current index after removal
    it->last_removed = JDM_TRUE;
    return JDM_ARRAY_OK;
}

jdm_array_status jdm_array_iterator_add(jdm_array_iterator_t* it, u8* elem) {
    jdm_array_status s = jdm_array_add_at(it->arr, elem, it->current_index);
    if (s != JDM_ARRAY_OK) {
        return s;
    }
    it->current_index++; // Move past the newly added element
    it->last_removed = JDM_FALSE;
    return JDM_ARRAY_OK;
}

jdm_array_status jdm_array_iterator_replace(jdm_array_iterator_t* it, u8* elem) {
    if (it->current_index == 0) {
        JDM_LOG_ERROR("No element to replace, iterator is at the beginning");
        return JDM_ARRAY_ERR_INVALID_INDEX;
    }
    sz index_to_replace = it->current_index - 1;
    return jdm_array_replace_at(it->arr, elem, index_to_replace);
}

jdm_array_status jdm_array_iterator_peek(jdm_array_iterator_t* it, u8** elem) {
    if (it->current_index == 0) {
        JDM_LOG_ERROR("No element to peek, iterator is at the beginning");
        return JDM_ARRAY_ERR_INVALID_INDEX;
    }
    sz index_to_peek = it->current_index - 1;
    return jdm_array_peek(it->arr, elem, index_to_peek);
}

jdm_array_status jdm_array_iterator_reset(jdm_array_iterator_t* it) {
    it->current_index = 0;
    it->last_removed = JDM_FALSE;
    return JDM_ARRAY_OK;
}

sz jdm_array_iterator_index(jdm_array_iterator_t* it) {
    if (it->current_index == 0) {
        return JDM_ARRAY_ERR_INVALID_INDEX;
    }
    return it->current_index - 1;
}

jdm_array_zip_iterator_t jdm_array_zip_iterator_create(jdm_array_t* arr1, jdm_array_t* arr2) {
    return (jdm_array_zip_iterator_t) {
        .arr1 = arr1,
        .arr2 = arr2,
        .current_index = 0,
        .last_removed = JDM_FALSE
    };
}

jdm_array_status jdm_array_zip_iterator_next(jdm_array_zip_iterator_t* it, u8** elem1, u8** elem2) {
    if (it->current_index >= it->arr1->size || it->current_index >= it->arr2->size) {
        return JDM_ARRAY_ERR_INVALID_INDEX;
    }
    *elem1 = JDM_ARRAY_BUF_ADDR(it->arr1, it->current_index);
    *elem2 = JDM_ARRAY_BUF_ADDR(it->arr2, it->current_index);
    it->current_index++;
    it->last_removed = JDM_FALSE;
    return JDM_ARRAY_OK;
}

jdm_array_status jdm_array_zip_iterator_remove(jdm_array_zip_iterator_t* it, u8* out_elem1, u8* out_elem2) {
    if (it->last_removed) {
        JDM_LOG_ERROR("Cannot remove the same element twice in a row");
        return JDM_ARRAY_ERR_INVALID_INDEX;
    }
    if (it->current_index == 0) {
        JDM_LOG_ERROR("No element to remove, iterator is at the beginning");
        return JDM_ARRAY_ERR_INVALID_INDEX;
    }
    sz index_to_remove = it->current_index - 1;
    jdm_array_status s1 = jdm_array_remove_at(it->arr1, index_to_remove, out_elem1);
    jdm_array_status s2 = jdm_array_remove_at(it->arr2, index_to_remove, out_elem2);
    if (s1 != JDM_ARRAY_OK || s2 != JDM_ARRAY_OK) {
        return JDM_ARRAY_ERR_INVALID_INDEX;
    }
    it->current_index--; // Adjust current index after removal
    it->last_removed = JDM_TRUE;
    return JDM_ARRAY_OK;
}

jdm_array_status jdm_array_zip_iterator_add(jdm_array_zip_iterator_t* it, u8* elem1, u8* elem2) {
    jdm_array_status s1 = jdm_array_add_at(it->arr1, elem1, it->current_index);
    jdm_array_status s2 = jdm_array_add_at(it->arr2, elem2, it->current_index);
    if (s1 != JDM_ARRAY_OK || s2 != JDM_ARRAY_OK) {
        return JDM_ARRAY_ERR_ALLOC;
    }
    it->current_index++; // Move past the newly added elements
    it->last_removed = JDM_FALSE;
    return JDM_ARRAY_OK;
}

jdm_array_status jdm_array_zip_iterator_replace(jdm_array_zip_iterator_t* it, u8* elem1, u8* elem2) {
    if (it->current_index == 0) {
        JDM_LOG_ERROR("No element to replace, iterator is at the beginning");
        return JDM_ARRAY_ERR_INVALID_INDEX;
    }
    sz index_to_replace = it->current_index - 1;
    jdm_array_status s1 = jdm_array_replace_at(it->arr1, elem1, index_to_replace);
    jdm_array_status s2 = jdm_array_replace_at(it->arr2, elem2, index_to_replace);
    if (s1 != JDM_ARRAY_OK || s2 != JDM_ARRAY_OK) {
        return JDM_ARRAY_ERR_INVALID_INDEX;
    }
    return JDM_ARRAY_OK;
}

jdm_array_status jdm_array_zip_iterator_peek(jdm_array_zip_iterator_t* it, u8** elem1, u8** elem2) {
    if (it->current_index == 0) {
        JDM_LOG_ERROR("No element to peek, iterator is at the beginning");
        return JDM_ARRAY_ERR_INVALID_INDEX;
    }
    sz index_to_peek = it->current_index - 1;
    jdm_array_status s1 = jdm_array_peek(it->arr1, elem1, index_to_peek);
    jdm_array_status s2 = jdm_array_peek(it->arr2, elem2, index_to_peek);
    if (s1 != JDM_ARRAY_OK || s2 != JDM_ARRAY_OK) {
        return JDM_ARRAY_ERR_INVALID_INDEX;
    }
    return JDM_ARRAY_OK;
}

jdm_array_status jdm_array_zip_iterator_reset(jdm_array_zip_iterator_t* it) {
    it->current_index = 0;
    it->last_removed = JDM_FALSE;
    return JDM_ARRAY_OK;
}

sz jdm_array_zip_iterator_index(jdm_array_zip_iterator_t* it) {
    if (it->current_index == 0) {
        return JDM_ARRAY_ERR_INVALID_INDEX;
    }
    return it->current_index - 1;
}
