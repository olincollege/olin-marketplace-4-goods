/**
 * String array library header.
 *
 * @author Steve Matsumoto
 * @date 2023
 * @copyright GNU GPLv3
 */
#pragma once

#include <stddef.h>
#include <stdlib.h>

// An array of strings that supports easier iteration and cleaner memory
// management. Internally, this is stored as an array of strings (char**), but
// with some extra bookkeeping data. In the internal array of strings, a null
// pointer acts as a sentinel value to indicate the end of strings in the array.
typedef struct {
  /// The raw array of strings.
  char** strings;
  /// The number of strings in the array.
  size_t size;
  /// The number of strings that the array can hold before needing to be
  /// resized.
  size_t capacity;
} string_array;

/**
 * Make a new string array with a given capacity.
 *
 * Create a new string array on the heap and return a pointer to it. The caller
 * is responsible for cleaning up the memory allocated for the array.
 *
 * @param capacity The starting capacity of the new string array.
 * @return The address of the new string array.
 */
string_array* make_string_array(size_t capacity);

/**
 * Free the memory allocated for a string array.
 *
 * Given a pointer to a string array, free the dynamic memory allocated for that
 * array. Attempting to free a pointer not to a string array, freeing a
 * string_array* that has already been freed, or freeing an unallocated
 * string_array* will result in undefined behavior.
 *
 * @param array The pointer to the array to free.
 */
void free_string_array(string_array* array);

/**
 * Resize a string array to a given capacity.
 *
 * Given a string array and a new capacity, set the capacity of the array to the
 * new value. If the new capacity is at least the current capacity of the array,
 * then the current strings in the array and its size are left unchanged. Since
 * memory is reallocated, the raw array of strings (array->strings) may be
 * moved or copied to a new pointer and the old pointer is not guaranteed to
 * remain usable.
 *
 * If the new capacity is less than the current capacity of the array, the last
 * strings in the array exceeding the capacity will be freed and removed from
 * the array. The size and capacity of the array will be also be reduced
 * accordingly. The array retains its sentinel null pointer, which is excluded
 * from the overall capacity.
 *
 * @param array A pointer to the array to resize.
 * @param capacity The new capacity.
 */
void resize(string_array* array, size_t capacity);

/**
 * Add a string to the end of an array, possibly resizing the array.
 *
 * Given a string array, a new string, and the size of that string, append a
 * copy of the string to the array. The new string will be the last one in the
 * array and will be followed by a sentinel null pointer. The size argument is
 * only the length of the string itself and does not include the null
 * terminator; however, the newly appended string will contain the null
 * terminator at the end. The memory for the new string is dynamically allocated
 * and managed by the array (i.e., the array will take care of cleaning it up).
 *
 * If the size of the array before adding the new string is the capacity of the
 * array, then the array is resized before the append. The new capacity of the
 * array is 1 (if the array's capacity is 0) or double the current capacity (if
 * the array's capacity is positive). After successfully executing, the size of
 * the array will also be incremented by 1.
 *
 * @param array A pointer to the array to append the string to.
 * @param string The string to add to the array.
 * @param size The maximum number of characters to add (no null terminator).
 */
void add_string(string_array* array, const char* string, size_t size);
