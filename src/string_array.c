/**
 * String array library implementation.
 *
 * @author Steve Matsumoto
 * @date 2023
 * @copyright GNU GPLv3
 */
#include "string_array.h"

#include <stddef.h>
#include <stdlib.h>

string_array* make_string_array(size_t capacity) {
  string_array* array = malloc(sizeof(string_array));
  array->size = 0;
  array->capacity = capacity;
  // Add space for the sentinel null pointer, which isn't part of the capacity.
  array->strings = malloc((capacity + 1) * sizeof(char*));
  array->strings[0] = NULL;
  return array;
}

void free_string_array(string_array* array) {
  // There's one more string in array->strings than the size, but it's a null
  // pointer, so no need to free it.
  for (size_t i = 0; i < array->size; ++i) {
    free(array->strings[i]);
  }
  free(array->strings);
  free(array);
}

void resize(string_array* array, size_t capacity) {
  if (capacity < array->size) {
    // Delete any strings over the new capacity.
    for (size_t i = capacity; i < array->size; ++i) {
      free(array->strings[i]);
    }
    array->size = capacity;
  }
  array->capacity = capacity;
  array->strings = realloc(array->strings, (capacity + 1) * sizeof(char*));
  array->strings[array->capacity] = NULL;
}

void add_string(string_array* array, const char* string, size_t size) {
  if (array->size == array->capacity) {
    resize(array, array->capacity ? array->capacity * 2 : 1);
  }
  // The slot to add to is always the sentinel null pointer, so we can just
  // malloc over it. Add space for the null terminator.
  array->strings[array->size] = malloc((size + 1) * sizeof(char));
  for (size_t i = 0; i < size; ++i) {
    array->strings[array->size][i] = string[i];
    if (string[i] == '\0') {
      break;
    }
  }
  // The null terminator may have already been added earlier, but still add it
  // here, since the memory has been allocated and we may have gone through the
  // whole string already.
  array->strings[array->size][size] = '\0';
  // Resize the array and put the sentinel null pointer in the right place.
  ++array->size;
  array->strings[array->size] = NULL;
}
