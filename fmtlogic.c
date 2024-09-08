/// @file fmtlogic.c
/// @brief Utility logic for formatting TOML structures.
#include "fmtlogic.h"

#include <assert.h>
#include <string.h>

/// @def MAX_INLINE_TABLE_SIZE
/// @brief The maximum size of a table that can be compacted into a single line.
#define MAX_INLINE_TABLE_SIZE 5

/// @def MAX_INLINE_STRING_SIZE
/// @brief The max size of a string that can be present in a single line table.
#define MAX_INLINE_STRING_SIZE 40

/// @def MAX_INLINE_ARRAY_SIZE
/// @brief The maximum size of an array that can be compacted into a line.
#define MAX_INLINE_ARRAY_SIZE 3

bool toml_table_can_be_compacted(const toml_table_t* tab,
                                 const bool checkStringLength) {
  for (int i = 0;; i++) {
    const char* k = toml_key_in(tab, i);
    if (k == NULL) break;

    const toml_table_t* vt = toml_table_in(tab, k);
    const toml_array_t* va = toml_array_in(tab, k);
    char* vs = NULL;

    if (vt != NULL) {
      if (!toml_table_can_be_compacted(vt, checkStringLength)) return false;
    } else if (va != NULL) {
      if (!toml_array_can_be_compacted(va)) return false;
    } else if (checkStringLength && !toml_rtos(toml_raw_in(tab, k), &vs)) {
      if (strlen(vs) >= MAX_INLINE_STRING_SIZE)
        return false;// string too long to inline
    }

    if (i > MAX_INLINE_TABLE_SIZE) return false;
  }
  return true;
}

bool toml_array_can_be_compacted(const toml_array_t* arr) {
  const int size = toml_array_nelem(arr);
  if (size == 0) return 1;                   // empty arrays always inline
  if (toml_array_is_primitive(arr)) return 1;// primitive arrays always inline

  for (int i = 0; i < size; i++) {
    const toml_table_t* tab = toml_table_at(arr, i);
    assert(tab != NULL);
    if (!toml_table_can_be_compacted(tab, false)) return false;
  }
  return true;
}

bool toml_array_should_use_newline_between_elements(const toml_array_t* arr) {
  if (toml_array_is_primitive(arr))
    return false;// primitive arrays always inline

  const int size = toml_array_nelem(arr);
  if (size > MAX_INLINE_ARRAY_SIZE) return true;// too many elements

  for (int i = 0; i < size; i++) {// check if all tables can be inlined
    const toml_table_t* tab = toml_table_at(arr, i);
    assert(tab != NULL);
    if (!toml_table_can_be_compacted(tab, true)) return true;
  }

  return false;
}

bool toml_array_is_primitive(const toml_array_t* arr) {
  return toml_array_type(arr) != 0;
}
