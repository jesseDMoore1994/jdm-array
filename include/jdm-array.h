#ifndef JDM_ARRAY_H
#define JDM_ARRAY_H

#include <jdm-defs.h>
#include <jdm-alloc.h>

// Status codes for jdm_array operations
typedef enum {
    JDM_ARRAY_OK = 0,
    JDM_ARRAY_ERR_MAX_CAP,
    JDM_ARRAY_ERR_ALLOC,
    JDM_ARRAY_ERR_INVALID_INDEX,
    JDM_ARRAY_ERR_NOT_FOUND,
} jdm_array_status;


// The primary array structure. This is an opaque type, and its internal
// details are hidden from the user. The user interacts with the array through
// the provided functions.
typedef struct jdm_array_t jdm_array_t;
typedef struct {
    jdm_alloc_t* alloc;  // allocator to use for memeory
    sz capacity;         // number of elements in array
    sz element_size;     // defaults to size of pointer
    f32 exp_factor;      // how much to grow the memory by if supported by allocator
} jdm_array_config_t;

typedef void (*jdm_array_print_elem_func_t)(u8* elem, sz index);
typedef int (*jdm_array_compare_func_t)(const pointer a, const pointer b);
typedef void (*jdm_array_map_func_t)(u8* elem);
typedef void (*jdm_array_reduce_func_t)(u8* elem, pointer user_data);
typedef b8 (*jdm_array_predicate_func_t)(const pointer elem);

jdm_array_t* jdm_array_create(const jdm_array_config_t cfg);
void jdm_array_destroy(jdm_array_t* arr);
void jdm_array_print(jdm_array_t* arr, sz elem_size, jdm_array_print_elem_func_t print_elem);
jdm_array_status jdm_array_add(jdm_array_t*arr, u8* elem);
jdm_array_status jdm_array_add_at(jdm_array_t*arr, u8* elem, sz index);
jdm_array_status jdm_array_replace_at(jdm_array_t*arr, u8* elem, sz index);
jdm_array_status jdm_array_swap_at(jdm_array_t*arr, sz index1, sz index2);
jdm_array_status jdm_array_remove(jdm_array_t*arr, u8* elem);
jdm_array_status jdm_array_remove_at(jdm_array_t*arr, sz index, u8* out_elem);
jdm_array_status jdm_array_remove_last(jdm_array_t*arr, u8* out_elem);
jdm_array_status jdm_array_remove_all(jdm_array_t*arr);
jdm_array_status jdm_array_get_at(jdm_array_t*arr, u8** elem, sz index);
jdm_array_status jdm_array_get_last(jdm_array_t*arr, u8** elem);
jdm_array_status jdm_array_peek(jdm_array_t*arr, u8** elem, sz index);
jdm_array_status jdm_array_index_of(jdm_array_t*arr, u8* elem, sz* out_index);
jdm_array_status jdm_array_subarray(jdm_array_t*arr, jdm_array_t** out_subarray, sz start_index, sz end_index);
jdm_array_status jdm_array_copy(jdm_array_t*arr, jdm_array_t** out_copy);
jdm_array_status jdm_array_reverse(jdm_array_t*arr);
jdm_array_status jdm_array_trim_capacity(jdm_array_t*arr);
jdm_array_status jdm_array_sort(jdm_array_t*arr, jdm_array_compare_func_t compare);
jdm_array_status jdm_array_map(jdm_array_t*arr, jdm_array_map_func_t map_func);
jdm_array_status jdm_array_reduce(jdm_array_t*arr, jdm_array_reduce_func_t reduce_func, pointer user_data);
jdm_array_status jdm_array_filter_mutable(jdm_array_t*arr, jdm_array_predicate_func_t predicate_func);
jdm_array_status jdm_array_filter(jdm_array_t*arr, jdm_array_t** out_filtered, jdm_array_predicate_func_t predicate_func);
sz jdm_array_count_of(jdm_array_t*arr, u8* elem);
sz jdm_array_element_size(jdm_array_t*arr);
sz jdm_array_size(jdm_array_t*arr);
sz jdm_array_capacity(jdm_array_t*arr);

// Iterator for jdm_array_t and its associated functions
typedef struct  {
    jdm_array_t* arr;
    sz current_index;
    b8 last_removed;
} jdm_array_iterator_t;

jdm_array_iterator_t jdm_array_iterator_create(jdm_array_t* arr);
jdm_array_status jdm_array_iterator_next(jdm_array_iterator_t* it, u8** elem);
jdm_array_status jdm_array_iterator_remove(jdm_array_iterator_t* it, u8* out_elem);
jdm_array_status jdm_array_iterator_add(jdm_array_iterator_t* it, u8* elem);
jdm_array_status jdm_array_iterator_replace(jdm_array_iterator_t* it, u8* elem);
jdm_array_status jdm_array_iterator_peek(jdm_array_iterator_t* it, u8** elem);
jdm_array_status jdm_array_iterator_reset(jdm_array_iterator_t* it);
sz jdm_array_iterator_index(jdm_array_iterator_t* it);


// Zip iterator for jdm_array_t and its associated functions
typedef struct {
    jdm_array_t* arr1;
    jdm_array_t* arr2;
    sz current_index;
    b8 last_removed;
} jdm_array_zip_iterator_t;

jdm_array_zip_iterator_t jdm_array_zip_iterator_create(jdm_array_t* arr1, jdm_array_t* arr2);
jdm_array_status jdm_array_zip_iterator_next(jdm_array_zip_iterator_t* it, u8** elem1, u8** elem2);
jdm_array_status jdm_array_zip_iterator_remove(jdm_array_zip_iterator_t* it, u8* out_elem1, u8* out_elem2);
jdm_array_status jdm_array_zip_iterator_add(jdm_array_zip_iterator_t* it, u8* elem1, u8* elem2);
jdm_array_status jdm_array_zip_iterator_replace(jdm_array_zip_iterator_t* it, u8* elem1, u8* elem2);
jdm_array_status jdm_array_zip_iterator_peek(jdm_array_zip_iterator_t* it, u8** elem1, u8** elem2);
jdm_array_status jdm_array_zip_iterator_reset(jdm_array_zip_iterator_t* it);
sz jdm_array_zip_iterator_index(jdm_array_zip_iterator_t* it);

#endif // JDM_ARRAY_H
