#include "jdm-array.h"
#include <jdm-log.h>
#include <string.h>

typedef struct {
  jdm_alloc_t* test_alloc;
  jdm_array_t* test_arr;
} test_ctx;

static test_ctx get_test_ctx(void){
  jdm_alloc_t* alloc = jdm_alloc_create(
    (jdm_alloc_config_t) {
      .type = JDM_ALLOC_SYSTEM
    }
  );
  jdm_array_t* arr = jdm_array_create(
    (jdm_array_config_t) {
      .alloc = alloc,
      .capacity = 1024,
      .element_size = sizeof(int),
      .exp_factor = 2,
    }
  );
  return (test_ctx) {
    .test_alloc = alloc,
    .test_arr = arr,
  };
}

static test_ctx get_test_ctx_with_str(void){
  jdm_alloc_t* alloc = jdm_alloc_create(
    (jdm_alloc_config_t) {
      .type = JDM_ALLOC_SYSTEM
    }
  );
  jdm_array_t* arr = jdm_array_create(
    (jdm_array_config_t) {
      .alloc = alloc,
      .capacity = 1024,
      .element_size = sizeof(char*),
      .exp_factor = 2,
    }
  );
  return (test_ctx) {
    .test_alloc = alloc,
    .test_arr = arr,
  };
}

static test_ctx get_populated_test_ctx(void) {
  test_ctx ctx = get_test_ctx();
  jdm_array_add(ctx.test_arr, (u8*)&(int){1});
  jdm_array_add(ctx.test_arr, (u8*)&(int){2});
  jdm_array_add(ctx.test_arr, (u8*)&(int){3});
  return ctx;
}

static void destroy_test_ctx(test_ctx ctx) {
  jdm_array_destroy(ctx.test_arr);
  jdm_alloc_destroy(ctx.test_alloc);
}

static void print_int_elem(u8* elem, sz index) {
  int* int_elem = (int*)elem;
  JDM_LOG_INFO("Element[%zu]: %d", index, *int_elem);
}

static b8 test_array_ops(void) {
  jdm_array_status_t s;
  test_ctx ctx = get_test_ctx();

  jdm_alloc_t* alloc = ctx.test_alloc;
  if(alloc == JDM_NULL) return JDM_FALSE;
  jdm_array_t* arr = ctx.test_arr;
  if(arr == JDM_NULL) return JDM_FALSE;

  // verify that the array is empty and has the correct capacity
  if(jdm_array_size(arr) != 0) {
    JDM_LOG_ERROR("Expected array size to be 0");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  if(jdm_array_capacity(arr) != 1024) {
    JDM_LOG_ERROR("Expected array capacity to be 1024");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  // Add elements to the array
  s = jdm_array_add(arr, (u8*)&(int){42});
  if(s != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to add element to array");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  s = jdm_array_add(arr, (u8*)&(int){43});
  if(s != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to add element to array");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  s = jdm_array_add(arr, (u8*)&(int){44});
  if(s != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to add element to array");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  // verify that the elements were added correctly
  int* elem = jdm_alloc(alloc, sizeof(int));
  if(elem == JDM_NULL) {
    JDM_LOG_ERROR("Failed to allocate memory for element");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  s = jdm_array_get_at(arr, (u8**)&elem, 1);
  if(s != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to get element from array");
    jdm_free(alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  if(*elem != 43) {
    JDM_LOG_ERROR("Element at index 1 is not 43");
    jdm_free(alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  // Replace element at index 1 with 99
  s = jdm_array_replace_at(arr, (u8*)&(int){99}, 1);
  if(s != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to replace element in array");
    jdm_free(alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  s = jdm_array_get_at(arr, (u8**)&elem, 1);
  if(s != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to get element from array");
    jdm_free(alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  if(*elem != 99) {
    JDM_LOG_ERROR("Element at index 1 is not 99 after replacement");
    jdm_free(alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  // Swap elements at index 0 and 2
  s = jdm_array_swap_at(arr, 0, 2);
  if(s != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to swap elements in array");
    jdm_free(alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  s = jdm_array_get_at(arr, (u8**)&elem, 0);
  if(s != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to get element from array");
    jdm_free(alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  if(*elem != 44) {
    JDM_LOG_ERROR("Element at index 0 is not 44 after swap");
    jdm_free(alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  s = jdm_array_get_at(arr, (u8**)&elem, 2);
  if(s != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to get last element from array");
    jdm_free(alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  if(*elem != 42) {
    JDM_LOG_ERROR("Last element is not 42 after swap");
    JDM_LOG_ERROR("Last element is %d", *elem);
    jdm_free(alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  // Remove element at index 1
  s = jdm_array_remove_at(arr, 1, JDM_NULL);
  if(s != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to remove element from array");
    jdm_free(alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  s = jdm_array_get_at(arr, (u8**)&elem, 1);
  if(s != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to get element from array after removal");
    jdm_free(alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  if(*elem != 42) {
    JDM_LOG_ERROR("Element at index 1 is not 42 after removal");
    jdm_free(alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  // remove by value
  s = jdm_array_remove(arr, (u8*)&(int){44});
  if(s != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to remove element by value from array");
    jdm_free(alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  s = jdm_array_get_at(arr, (u8**)&elem, 0);
  if(s != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to get element from array after removal by value");
    jdm_free(alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  if(*elem != 42) {
    JDM_LOG_ERROR("Element at index 0 is not 42 after removal by value");
    jdm_free(alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  // remove last element
  s = jdm_array_remove_last(arr, JDM_NULL);
  if(s != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to remove last element from array");
    jdm_free(alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  s = jdm_array_get_at(arr, (u8**)&elem, 0);
  if(s != JDM_ARRAY_ERR_INVALID_INDEX) {
    JDM_LOG_ERROR(" Expected error when getting element at index 0 after removing last element");
    jdm_free(alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  // add some more elements and remove all
  jdm_array_add(arr, (u8*)&(int){1});
  jdm_array_add(arr, (u8*)&(int){2});
  jdm_array_add(arr, (u8*)&(int){3});
  if(jdm_array_size(arr) != 3) {
    JDM_LOG_ERROR("Expected array size to be 3 after adding 3 elements");
    jdm_free(alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  s = jdm_array_remove_all(arr);
  if(s != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to remove all elements from array");
    jdm_free(alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  s = jdm_array_get_at(arr, (u8**)&elem, 0);
  if(s != JDM_ARRAY_ERR_INVALID_INDEX) {
    JDM_LOG_ERROR("Expected error when getting element at index 0 after removing all elements");
    jdm_free(alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  if(jdm_array_size(arr) != 0) {
    JDM_LOG_ERROR("Expected array size to be 0 after removing all elements");
    jdm_free(alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  // tests passed. cleanup and return success
  jdm_free(alloc, elem);
  destroy_test_ctx(ctx);
  return JDM_TRUE;
}

static b8 test_array_peek(void) {
  test_ctx ctx = get_populated_test_ctx();
  jdm_array_t* arr = ctx.test_arr;
  int* elem = NULL;
  jdm_array_status_t s;

  // peek at index 1 (notice that peek is shallow, it passes the pointer to the element, not a copy of the element)
  s = jdm_array_peek(arr, (u8**)&elem, 1);
  if(s != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to peek at index 1");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  if(*elem != 2) {
    JDM_LOG_ERROR("Peeked element at index 1 is not 2");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  // peek at index out of bounds
  s = jdm_array_peek(arr, (u8**)&elem, 3);
  if(s != JDM_ARRAY_ERR_INVALID_INDEX) {
    JDM_LOG_ERROR("Expected error when peeking at index out of bounds");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  destroy_test_ctx(ctx);
  return JDM_TRUE;
}

static b8 test_array_subarray(void) {
  test_ctx ctx = get_populated_test_ctx();
  jdm_array_t* arr = ctx.test_arr;
  jdm_array_t* subarr = NULL;
  jdm_array_status_t s;

  // create a subarray from index 0 to 2 (should contain elements 1 and 2)
  s = jdm_array_subarray(arr, &subarr, 0, 2);
  if(s != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to create subarray");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  if(jdm_array_size(subarr) != 2) {
    JDM_LOG_ERROR("Subarray size is not 2");
    jdm_array_destroy(subarr);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  int* elem = jdm_alloc(ctx.test_alloc, sizeof(int));
  if(elem == JDM_NULL) {
    JDM_LOG_ERROR("Failed to allocate memory for element");
    jdm_array_destroy(subarr);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  s = jdm_array_get_at(subarr, (u8**)&elem, 0);
  if(s != JDM_ARRAY_OK || *elem != 1) {
    JDM_LOG_ERROR("Subarray element at index 0 is not 1");
    jdm_free(ctx.test_alloc, elem);
    jdm_array_destroy(subarr);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  s = jdm_array_get_at(subarr, (u8**)&elem, 1);
  if(s != JDM_ARRAY_OK || *elem != 2) {
    JDM_LOG_ERROR("Subarray element at index 1 is not 2");
    jdm_free(ctx.test_alloc, elem);
    jdm_array_destroy(subarr);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  // should not be able to get element at index 2
  s = jdm_array_get_at(subarr, (u8**)&elem, 2);
  if(s != JDM_ARRAY_ERR_INVALID_INDEX) {
    JDM_LOG_ERROR("Expected error when getting element at index 2 of subarray");
    jdm_free(ctx.test_alloc, elem);
    jdm_array_destroy(subarr);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  jdm_free(ctx.test_alloc, elem);
  jdm_array_destroy(subarr);
  destroy_test_ctx(ctx);
  return JDM_TRUE;
}

static b8 test_array_copy(void) {
  test_ctx ctx = get_populated_test_ctx();
  jdm_array_t* arr = ctx.test_arr;
  jdm_array_t* copy = NULL;
  jdm_array_status_t s;

  s = jdm_array_copy(arr, &copy);
  if(s != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to copy array");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  if(jdm_array_size(copy) != jdm_array_size(arr)) {
    JDM_LOG_ERROR("Copied array size does not match original");
    jdm_array_destroy(copy);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  int* elem = jdm_alloc(ctx.test_alloc, sizeof(int));
  if(elem == JDM_NULL) {
    JDM_LOG_ERROR("Failed to allocate memory for element");
    jdm_array_destroy(copy);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  int* elem_copy = jdm_alloc(ctx.test_alloc, sizeof(int));
  if(elem == JDM_NULL) {
    JDM_LOG_ERROR("Failed to allocate memory for element");
    jdm_free(ctx.test_alloc, elem);
    jdm_array_destroy(copy);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  for(sz i = 0; i < jdm_array_size(arr); i++) {
    s = jdm_array_get_at(copy, (u8**)&elem_copy, i);
    if(s != JDM_ARRAY_OK) {
      JDM_LOG_ERROR("Failed to get element at index %zu from copied array", i);
      jdm_free(ctx.test_alloc, elem);
      jdm_free(ctx.test_alloc, elem_copy);
      jdm_array_destroy(copy);
      destroy_test_ctx(ctx);
      return JDM_FALSE;
    }
    s = jdm_array_get_at(arr, (u8**)&elem, i);
    if(s != JDM_ARRAY_OK) {
      JDM_LOG_ERROR("Failed to get element at index %zu from original array", i);
      jdm_free(ctx.test_alloc, elem);
      jdm_free(ctx.test_alloc, elem_copy);
      jdm_array_destroy(copy);
      destroy_test_ctx(ctx);
      return JDM_FALSE;
    }
    if(*elem_copy != *elem) {
      JDM_LOG_ERROR("Element at index %zu in copied array does not match original", i);
      jdm_free(ctx.test_alloc, elem);
      jdm_free(ctx.test_alloc, elem_copy);
      jdm_array_destroy(copy);
      destroy_test_ctx(ctx);
      return JDM_FALSE;
    }
  }

  jdm_free(ctx.test_alloc, elem);
  jdm_free(ctx.test_alloc, elem_copy);
  jdm_array_destroy(copy);
  destroy_test_ctx(ctx);
  return JDM_TRUE;
}

static b8 test_array_reverse(void) {
  test_ctx ctx = get_populated_test_ctx();
  jdm_array_t* arr = ctx.test_arr;
  jdm_array_status_t s;

  s = jdm_array_reverse(arr);
  if(s != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to reverse array");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  int* elem = jdm_alloc(ctx.test_alloc, sizeof(int));
  if(elem == JDM_NULL) {
    JDM_LOG_ERROR("Failed to allocate memory for element");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  s = jdm_array_get_at(arr, (u8**)&elem, 0);
  if(s != JDM_ARRAY_OK || *elem != 3) {
    JDM_LOG_ERROR("Element at index 0 is not 3 after reverse");
    jdm_free(ctx.test_alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  s = jdm_array_get_at(arr, (u8**)&elem, 1);
  if(s != JDM_ARRAY_OK || *elem != 2) {
    JDM_LOG_ERROR("Element at index 1 is not 2 after reverse");
    jdm_free(ctx.test_alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  s = jdm_array_get_at(arr, (u8**)&elem, 2);
  if(s != JDM_ARRAY_OK || *elem != 1) {
    JDM_LOG_ERROR("Element at index 2 is not 1 after reverse");
    jdm_free(ctx.test_alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  jdm_free(ctx.test_alloc, elem);
  destroy_test_ctx(ctx);
  return JDM_TRUE;
}

static b8 test_array_trim_capacity(void) {
  test_ctx ctx = get_populated_test_ctx();
  jdm_array_t* arr = ctx.test_arr;
  jdm_array_status_t s;

  // trim capacity
  s = jdm_array_trim_capacity(arr);
  if(s != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to trim array capacity");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  if(jdm_array_capacity(arr) != jdm_array_size(arr)) {
    JDM_LOG_ERROR("Array capacity does not match size after trim");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  destroy_test_ctx(ctx);
  return JDM_TRUE;
}

static int compare_desc(const pointer a, const pointer b) {
  int* int_a = (int*)a;
  int* int_b = (int*)b;
  return *int_b - *int_a; // descending order
}

static b8 test_array_sort(void) {
  test_ctx ctx = get_populated_test_ctx();
  jdm_array_t* arr = ctx.test_arr;
  jdm_array_status_t s;

  // sort the array in descending order
  s = jdm_array_sort(arr, compare_desc);
  if(s != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to sort array");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  int* elem = jdm_alloc(ctx.test_alloc, sizeof(int));
  if(elem == JDM_NULL) {
    JDM_LOG_ERROR("Failed to allocate memory for element");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  s = jdm_array_get_at(arr, (u8**)&elem, 0);
  if(s != JDM_ARRAY_OK || *elem != 3) {
    JDM_LOG_ERROR("Element at index 0 is not 3 after sort");
    jdm_free(ctx.test_alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  s = jdm_array_get_at(arr, (u8**)&elem, 1);
  if(s != JDM_ARRAY_OK || *elem != 2) {
    JDM_LOG_ERROR("Element at index 1 is not 2 after sort");
    jdm_free(ctx.test_alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  s = jdm_array_get_at(arr, (u8**)&elem, 2);
  if(s != JDM_ARRAY_OK || *elem != 1) {
    JDM_LOG_ERROR("Element at index 2 is not 1 after sort");
    jdm_free(ctx.test_alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  jdm_free(ctx.test_alloc, elem);
  destroy_test_ctx(ctx);
  return JDM_TRUE;
}

static void test_array_map_func(u8* elem) {
  int* int_elem = (int*)elem;
  *int_elem *= 2; // double the value
}

static b8 test_array_map(void) {
  test_ctx ctx = get_populated_test_ctx();
  jdm_array_t* arr = ctx.test_arr;
  jdm_array_status_t s;

  // map the array to double each element
  s = jdm_array_map(arr, test_array_map_func);
  if(s != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to map array");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  int* elem = jdm_alloc(ctx.test_alloc, sizeof(int));
  if(elem == JDM_NULL) {
    JDM_LOG_ERROR("Failed to allocate memory for element");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  s = jdm_array_get_at(arr, (u8**)&elem, 0);
  if(s != JDM_ARRAY_OK || *elem != 2) {
    JDM_LOG_ERROR("Element at index 0 is not 2 after map");
    jdm_free(ctx.test_alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  s = jdm_array_get_at(arr, (u8**)&elem, 1);
  if(s != JDM_ARRAY_OK || *elem != 4) {
    JDM_LOG_ERROR("Element at index 1 is not 4 after map");
    jdm_free(ctx.test_alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  s = jdm_array_get_at(arr, (u8**)&elem, 2);
  if(s != JDM_ARRAY_OK || *elem != 6) {
    JDM_LOG_ERROR("Element at index 2 is not 6 after map");
    jdm_free(ctx.test_alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  jdm_free(ctx.test_alloc, elem);
  destroy_test_ctx(ctx);
  return JDM_TRUE;
}

static void test_array_reduce_func(u8* elem, pointer accumulator) {
  int* int_elem = (int*)elem;
  int* acc = (int*)accumulator;
  *acc += *int_elem; // sum the values
}

static b8 test_array_reduce(void) {
  test_ctx ctx = get_populated_test_ctx();
  jdm_array_t* arr = ctx.test_arr;
  jdm_array_status_t s;

  int sum = 0;
  s = jdm_array_reduce(arr, test_array_reduce_func, (u8*)&sum);
  if(s != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to reduce array");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  if(sum != 6) { // 1 + 2 + 3 = 6
    JDM_LOG_ERROR("Sum of elements is not 6 after reduce");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  destroy_test_ctx(ctx);
  return JDM_TRUE;
}

static b8 test_array_is_even_predicate(const pointer elem) {
  int* int_elem = (int*)elem;
  return (*int_elem % 2) == 0;
}

static b8 test_array_filter_mutable(void) {
  test_ctx ctx = get_populated_test_ctx();
  jdm_array_t* arr = ctx.test_arr;
  jdm_array_status_t s;

  // filter the array to keep only even numbers
  s = jdm_array_filter_mutable(arr, test_array_is_even_predicate);
  if(s != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to filter array");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  if(jdm_array_size(arr) != 1) { // only '2' should remain
    JDM_LOG_ERROR("Array size is not 1 after filter");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  int* elem = jdm_alloc(ctx.test_alloc, sizeof(int));
  if(elem == JDM_NULL) {
    JDM_LOG_ERROR("Failed to allocate memory for element");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  s = jdm_array_get_at(arr, (u8**)&elem, 0);
  if(s != JDM_ARRAY_OK || *elem != 2) {
    JDM_LOG_ERROR("Element at index 0 is not 2 after filter");
    jdm_free(ctx.test_alloc, elem);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  jdm_free(ctx.test_alloc, elem);
  destroy_test_ctx(ctx);
  return JDM_TRUE;
}

static b8 test_array_filter(void) {
  test_ctx ctx = get_populated_test_ctx();
  jdm_array_t* arr = ctx.test_arr;
  jdm_array_t* filtered_arr = NULL;
  jdm_array_status_t s;

  // filter the array to keep only even numbers
  s = jdm_array_filter(arr, &filtered_arr, test_array_is_even_predicate);
  if(s != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to filter array");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  if(jdm_array_size(filtered_arr) != 1) { // only '2' should remain
    JDM_LOG_ERROR("Filtered array size is not 1 after filter");
    jdm_array_destroy(filtered_arr);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  int* elem = jdm_alloc(ctx.test_alloc, sizeof(int));
  if(elem == JDM_NULL) {
    JDM_LOG_ERROR("Failed to allocate memory for element");
    jdm_array_destroy(filtered_arr);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  s = jdm_array_get_at(filtered_arr, (u8**)&elem, 0);
  if(s != JDM_ARRAY_OK || *elem != 2) {
    JDM_LOG_ERROR("Element at index 0 in filtered array is not 2 after filter");
    jdm_free(ctx.test_alloc, elem);
    jdm_array_destroy(filtered_arr);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  jdm_free(ctx.test_alloc, elem);
  jdm_array_destroy(filtered_arr);
  destroy_test_ctx(ctx);
  return JDM_TRUE;
}

static b8 test_array_iterator(void) {
  test_ctx ctx = get_populated_test_ctx();
  jdm_array_t* arr = ctx.test_arr;
  jdm_array_iterator_t it = jdm_array_iterator_create(arr);
  u8* elem = NULL;
  sz index = 0;

  // test that the iterator iterates over all elements in order
  while(jdm_array_iterator_next(&it, &elem) == JDM_ARRAY_OK) {
    int* int_elem = (int*)elem;
    if(*int_elem != (int)(index + 1)) {
      JDM_LOG_ERROR("Iterator element at index %zu is not %d", index, (int)(index + 1));
      destroy_test_ctx(ctx);
      return JDM_FALSE;
    }
    index++;
  }

  if(index != jdm_array_size(arr)) {
    JDM_LOG_ERROR("Iterator did not iterate over all elements");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  // Test that after reset, we can iterate again
  if(jdm_array_iterator_reset(&it) != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to reset iterator");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  index = 0;
  while(jdm_array_iterator_next(&it, &elem) == JDM_ARRAY_OK) {
    int* int_elem = (int*)elem;
    if(*int_elem != (int)(index + 1)) {
      JDM_LOG_ERROR("Iterator element at index %zu after reset is not %d", index, (int)(index + 1));
      destroy_test_ctx(ctx);
      return JDM_FALSE;
    }
    index++;
  }

  // Test that removing an element via iterator works
  if(jdm_array_iterator_reset(&it) != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to reset iterator for removal test");
  }

  if(jdm_array_iterator_next(&it, &elem) != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to get first element for removal test");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  if(jdm_array_iterator_remove(&it, JDM_NULL) != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to remove first element via iterator");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  if(jdm_array_size(arr) != 2) {
    JDM_LOG_ERROR("Array size is not 2 after removing an element via iterator");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  // Test that you can replace an element via iterator
  if(jdm_array_iterator_reset(&it) != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to reset iterator for replacement test");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  if(jdm_array_iterator_next(&it, &elem) != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to get first element for replacement test");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  if(jdm_array_iterator_replace(&it, (u8*)&(int){99}) != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to replace first element via iterator");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  int* int_elem = NULL;
  if(jdm_array_peek(arr, (u8**)&int_elem, 0) != JDM_ARRAY_OK || *int_elem != 99) {
    JDM_LOG_ERROR("First element is not 99 after replacement via iterator");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  // Test that you can add an element via iterator
  if(jdm_array_iterator_add(&it, (u8*)&(int){100}) != JDM_ARRAY_OK) {
    JDM_LOG_ERROR("Failed to add element via iterator");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  if(jdm_array_size(arr) != 3) {
    JDM_LOG_ERROR("Array size is not 3 after adding an element via iterator");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  if(jdm_array_peek(arr, (u8**)&int_elem, 1) != JDM_ARRAY_OK || *int_elem != 100) {
    JDM_LOG_ERROR("Element at index 1 is not 100 after adding via iterator");
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  destroy_test_ctx(ctx);
  return JDM_TRUE;
}

static b8 test_array_zip_iterator(void) {
  test_ctx ctx1 = get_populated_test_ctx();
  test_ctx ctx2 = get_populated_test_ctx();
  jdm_array_t* arr1 = ctx1.test_arr;
  jdm_array_t* arr2 = ctx2.test_arr;

  jdm_array_zip_iterator_t zip_it = jdm_array_zip_iterator_create(arr1, arr2);
  u8* elem1 = NULL;
  u8* elem2 = NULL;
  sz index = 0;

  while(jdm_array_zip_iterator_next(&zip_it, &elem1, &elem2) == JDM_ARRAY_OK) {
    int* int_elem1 = (int*)elem1;
    int* int_elem2 = (int*)elem2;
    if(*int_elem1 != (int)(index + 1) || *int_elem2 != (int)(index + 1)) {
      JDM_LOG_ERROR("Zip iterator elements at index %zu are not equal to %d", index, (int)(index + 1));
      destroy_test_ctx(ctx1);
      destroy_test_ctx(ctx2);
      return JDM_FALSE;
    }
    index++;
  }

  if(index != jdm_array_size(arr1) || index != jdm_array_size(arr2)) {
    JDM_LOG_ERROR("Zip iterator did not iterate over all elements");
    destroy_test_ctx(ctx1);
    destroy_test_ctx(ctx2);
    return JDM_FALSE;
  }

  destroy_test_ctx(ctx1);
  destroy_test_ctx(ctx2);
  return JDM_TRUE;
}

static b8 test_array_pointer(void) {
  test_ctx ctx = get_test_ctx_with_str();
  jdm_array_t* arr = ctx.test_arr;
  int* elem = NULL;
  jdm_array_status_t s;

  char* str1 = jdm_calloc(ctx.test_alloc, 6, sizeof(char));
  strcpy(str1, "test1");
  JDM_LOG_ERROR("str1 (%p)", str1);
  char* str2 = jdm_calloc(ctx.test_alloc, 6, sizeof(char));
  strcpy(str2, "test2");
  JDM_LOG_ERROR("str2 (%p)", str2);
  char* str3 = jdm_calloc(ctx.test_alloc, 6, sizeof(char));
  strcpy(str3, "test3");
  JDM_LOG_ERROR("str3 (%p)", str3);

  jdm_array_add(arr, (u8*)&str1);
  jdm_array_add(arr, (u8*)&str2);
  jdm_array_add(arr, (u8*)&str3);

  char** out = jdm_alloc(ctx.test_alloc, sizeof(char*));
  *out = JDM_NULL;
  jdm_array_get_at(arr, (u8**)&out, 0);
  if(0 != strcmp(str1, *out)) {
    JDM_LOG_ERROR("pointer get at failed");
    JDM_LOG_ERROR("%s, %s", str1, out);
    jdm_free(ctx.test_alloc, str1);
    jdm_free(ctx.test_alloc, str2);
    jdm_free(ctx.test_alloc, str3);
    jdm_free(ctx.test_alloc, out);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  *out = JDM_NULL;
  jdm_array_get_at(arr, (u8**)&out, 1);
  if(0 != strcmp(str2, *out)) {
    JDM_LOG_ERROR("pointer get at failed");
    JDM_LOG_ERROR("%s, %s", str2, out);
    jdm_free(ctx.test_alloc, str1);
    jdm_free(ctx.test_alloc, str2);
    jdm_free(ctx.test_alloc, str3);
    jdm_free(ctx.test_alloc, out);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }
  *out = JDM_NULL;
  jdm_array_get_at(arr, (u8**)&out, 2);
  if(0 != strcmp(str3, *out)) {
    JDM_LOG_ERROR("pointer get at failed");
    JDM_LOG_ERROR("%s, %s", str3, out);
    jdm_free(ctx.test_alloc, str1);
    jdm_free(ctx.test_alloc, str2);
    jdm_free(ctx.test_alloc, str3);
    jdm_free(ctx.test_alloc, out);
    destroy_test_ctx(ctx);
    return JDM_FALSE;
  }

  jdm_free(ctx.test_alloc, str1);
  jdm_free(ctx.test_alloc, str2);
  jdm_free(ctx.test_alloc, str3);
  jdm_free(ctx.test_alloc, out);
  destroy_test_ctx(ctx);
  return JDM_TRUE;
}

int main() {
  jdm_log_init();
  JDM_LOG_NOTICE("Starting array tests");
  if(test_array_ops() == JDM_FALSE) {
    JDM_LOG_ERROR("Array operations test failed");
    return 1;
  }

  JDM_LOG_NOTICE("Starting array peek test");
  if(test_array_peek() == JDM_FALSE) {
    JDM_LOG_ERROR("Array peek test failed");
    return 1;
  }

  JDM_LOG_NOTICE("Starting array subarray test");
  if(test_array_subarray() == JDM_FALSE) {
    JDM_LOG_ERROR("Array subarray test failed");
    return 1;
  }

  JDM_LOG_NOTICE("Starting array copy test");
  if(test_array_copy() == JDM_FALSE) {
    JDM_LOG_ERROR("Array copy test failed");
    return 1;
  }

  JDM_LOG_NOTICE("Starting array reverse test");
  if(test_array_reverse() == JDM_FALSE) {
    JDM_LOG_ERROR("Array reverse test failed");
    return 1;
  }

  JDM_LOG_NOTICE("Starting array trim capacity test");
  if(test_array_trim_capacity() == JDM_FALSE) {
    JDM_LOG_ERROR("Array trim capacity test failed");
    return 1;
  }

  JDM_LOG_NOTICE("Starting array sort test");
  if(test_array_sort() == JDM_FALSE) {
    JDM_LOG_ERROR("Array sort test failed");
    return 1;
  }

  JDM_LOG_NOTICE("Starting array map test");
  if(test_array_map() == JDM_FALSE) {
    JDM_LOG_ERROR("Array map test failed");
    return 1;
  }

  JDM_LOG_NOTICE("Starting array reduce test");
  if(test_array_reduce() == JDM_FALSE) {
    JDM_LOG_ERROR("Array reduce test failed");
    return 1;
  }

  JDM_LOG_NOTICE("Starting array filter mutable test");
  if(test_array_filter_mutable() == JDM_FALSE) {
    JDM_LOG_ERROR("Array filter mutable test failed");
    return 1;
  }

  JDM_LOG_NOTICE("Starting array filter test");
  if(test_array_filter() == JDM_FALSE) {
    JDM_LOG_ERROR("Array filter test failed");
    return 1;
  }

  JDM_LOG_NOTICE("Starting array iterator test");
  if(test_array_iterator() == JDM_FALSE) {
    JDM_LOG_ERROR("Array iterator test failed");
    return 1;
  }

  JDM_LOG_NOTICE("Starting array zip iterator test");
  if(test_array_zip_iterator() == JDM_FALSE) {
    JDM_LOG_ERROR("Array zip iterator test failed");
    return 1;
  }

  JDM_LOG_NOTICE("Starting array pointer test");
  if(test_array_pointer() == JDM_FALSE) {
    JDM_LOG_ERROR("Array pointer test failed");
    return 1;
  }

  JDM_LOG_NOTICE("All tests passed");
  jdm_log_cleanup();
}
