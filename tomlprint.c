/// @file tomlprint.c
/// @brief Functions for formatting and printing TOML structures to a stream.
#include "tomlprint.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "fmtlogic.h"

/// @def ts_printf
/// @brief Writes a formatted string to the TOML stream.
/// @param ts The TOML stream to write to.
/// @param fmt The format string.
/// @param ... The format arguments.
#define ts_printf(ts, fmt, ...) ts->printf(ts->udata, fmt, __VA_ARGS__)

/// @def ts_puts
/// @brief Writes a string to the TOML stream.
/// @param ts The TOML stream to write to.
/// @param s The string to write.
#define ts_puts(ts, s) ts->puts(ts->udata, s)

/// @def ts_putc
/// @brief Writes a single character to the TOML stream.
/// @param ts The TOML stream to write to.
/// @param c The character to write.
#define ts_putc(ts, c) ts->putc(ts->udata, c)

/// @brief Prints a TOML string key to the stream. The string is enclosed in
/// double quotes if it contains spaces or special characters.
/// @param ts The TOML stream to print to.
/// @param s The string to print.
static void toml_print_key(const TOML* ts, const char* s) {
  for (int i = 0; s[i] != '\0'; i++) {
    switch (s[i]) {
      case ' ':
      case '\"':
      case '\'':
        ts_printf(ts, "\"%s\"", s);
        return;
    }
  }
  ts_puts(ts, s);
}

/// @brief Prints a TOML table header to the stream. The table name is enclosed
/// in single square brackets.
/// @param ts The TOML stream to print to.
/// @param s The table name to print.
static void toml_print_table_header(const TOML* ts, const char* s) {
  ts_putc(ts, '[');
  toml_print_key(ts, s);
  ts_putc(ts, ']');
}

/// @brief Prints a TOML array header to the stream. The array name is enclosed
/// in double square brackets.
/// @param ts The TOML stream to print to.
/// @param s The array name to print.
void toml_print_array_header(const TOML* ts, const char* s) {
  ts_puts(ts, "[[");
  toml_print_key(ts, s);
  ts_puts(ts, "]]");
}

static const char* ESCAPE_CHARS[256] = {
        ['\"'] = "\\\"", ['\\'] = "\\\\", ['\a'] = "\\a",
        ['\b'] = "\\b",  ['\n'] = "\\n",  ['\t'] = "\\t",
};

/// @brief Prints a TOML string value to the stream, escaping special characters
/// as necessary and always enclosing the string in double quotes.
/// @param ts The TOML stream to print to.
/// @param s The string to print.
static void toml_print_string_value(const TOML* ts, const char* s) {
  ts_putc(ts, '"');
  for (size_t i = 0; s[i] != '\0'; i++) {
    const char* escape = ESCAPE_CHARS[(unsigned char) s[i]];
    if (escape != NULL) {
      ts_puts(ts, escape);
    } else if (iscntrl(s[i])) {
      ts_printf(ts, "\\%03o", s[i]);
    } else {
      ts_putc(ts, s[i]);
    }
  }
  ts_putc(ts, '"');
}

/// @brief Prints a TOML double value to the stream, using the default float
/// format specifier. If the value can be represented as an integer, it will be
/// printed as such.
/// @param ts The TOML stream to print to.
/// @param d The double value to print.
static void toml_print_double_value(const TOML* ts, const double d) {
  ts_printf(ts, "%f", d);
}

/// @brief Prints a TOML raw value to the stream, determining the type of the
/// value and printing it accordingly.
/// @param ts The TOML stream to print to.
/// @param raw The raw TOML value to print.
static void toml_print_raw_value(const TOML* ts, const toml_raw_t raw) {
  union {
    char* s;
    int64_t i;
    int b;
    double d;
  } u = {0};
  if (toml_rtos(raw, &u.s) == 0) {
    toml_print_string_value(ts, u.s);
  } else if (toml_rtoi(raw, &u.i) == 0) {
    ts_printf(ts, "%lld", u.i);
  } else if (toml_rtob(raw, &u.b) == 0) {
    ts_puts(ts, u.b ? "true" : "false");
  } else if (toml_rtod(raw, &u.d) == 0) {
    toml_print_double_value(ts, u.d);
  } else {
    assert("unknown raw toml value type");
  }
}

/// @brief Prints a TOML key-value pair assignment prefix to the stream. The key
/// is printed followed by an equals sign. No newline suffix is appended.
/// @param ts The TOML stream to print to.
/// @param s The key name.
static inline void toml_print_assignment(const TOML* ts, const char* s) {
  toml_print_key(ts, s);
  ts_puts(ts, " = ");
}

/// @brief Prints an indent to the stream, consisting of a repeat number of tab
/// characters.
/// @param ts The TOML stream to print to.
/// @param n The number of tabs to print.
static inline void toml_print_indent(const TOML* ts, int n) {
  while (n-- > 0) ts_putc(ts, '\t');
}

/// @brief Prints a key-value pair assignment where the value is an array which
/// can be inline printed. A newline suffix is always appended.
/// @param ts The TOML stream to print to.
/// @param arr The TOML array to print.
static void toml_print_array_value_inline(const TOML* ts,
                                          const toml_array_t* arr);

/// @brief Prints a key-value pair assignment where the value is a table which
/// can be inline printed. A newline suffix is always appended.
/// @param ts The TOML stream to print to.
/// @param tab The TOML table to print.
static void toml_print_table_value_inline(const TOML* ts,
                                          const toml_table_t* tab);

static void toml_print_table_value_inline(const TOML* ts,
                                          const toml_table_t* tab) {
  ts_putc(ts, '{');
  for (int i = 0;; i++) {
    const char* k = toml_key_in(tab, i);
    if (k == NULL) break;

    if (i > 0) ts_puts(ts, ", ");

    toml_print_assignment(ts, k);

    void* ud;
    if ((ud = (void*) toml_table_in(tab, k)) != NULL) {
      toml_print_table_value_inline(ts, ud);
    } else if ((ud = (void*) toml_array_in(tab, k)) != NULL) {
      toml_print_array_value_inline(ts, ud);
    } else if ((ud = (void*) toml_raw_in(tab, k)) != NULL) {
      toml_print_raw_value(ts, ud);
    } else {
      assert("unknown table value type");
    }
  }
  ts_putc(ts, '}');
}

static void toml_print_array_value_inline(const TOML* ts,
                                          const toml_array_t* arr) {
  ts_putc(ts, '[');
  for (int i = 0; i < toml_array_nelem(arr); i++) {
    if (i > 0) ts_puts(ts, ", ");

    void* ud;
    if ((ud = (void*) toml_table_at(arr, i)) != NULL) {
      toml_print_table_value_inline(ts, ud);
    } else if ((ud = (void*) toml_array_at(arr, i)) != NULL) {
      toml_print_array_value_inline(ts, ud);
    } else if ((ud = (void*) toml_raw_at(arr, i)) != NULL) {
      toml_print_raw_value(ts, ud);
    } else {
      assert("unknown array value type");
    }
  }
  ts_putc(ts, ']');
}

/// @brief Prints an indented key-value pair assignment where the value is a raw
/// TOML value, such as a string, integer, or boolean. A newline suffix is
/// always appended.
/// @param ts The TOML stream to print to.
/// @param depth The current indent depth.
/// @param name The key name.
/// @param raw The raw TOML value to print.
static void toml_print_raw_assignment(const TOML* ts, const int depth,
                                      const char* name, const toml_raw_t raw) {
  toml_print_indent(ts, depth);
  toml_print_assignment(ts, name);
  toml_print_raw_value(ts, raw);
  ts_putc(ts, '\n');
}

/// @brief Prints an indented key-value pair assignment where the value is a
/// TOML table which can be inline printed. A newline suffix is always appended.
/// @param ts The TOML stream to print to.
/// @param depth The current indent depth.
/// @param name The key name.
/// @param tab The TOML table to print.
static void toml_print_table_assignment(const TOML* ts, const int depth,
                                        const char* name,
                                        const toml_table_t* tab) {
  toml_print_indent(ts, depth);
  toml_print_assignment(ts, name);
  toml_print_table_value_inline(ts, tab);
  ts_putc(ts, '\n');
}

/// @brief Prints an indented key-value pair assignment where the value is a
/// TOML array which can be inline printed. A newline suffix is always appended.
/// The array may be printed as a single line if it is small enough, or as one
/// element per line if it is too large.
/// @param ts The TOML stream to print to.
/// @param depth The current indent depth.
/// @param name The key name.
/// @param arr The TOML array to print.
static void toml_print_array_assignment(const TOML* ts, const int depth,
                                        const char* name,
                                        const toml_array_t* arr) {
  toml_print_indent(ts, depth);
  toml_print_assignment(ts, name);
  ts_putc(ts, '[');

  // pre-scan the array to determine if it should be printed inline
  const bool requiresNewlines =
          toml_array_should_use_newline_between_elements(arr);

  const int size = toml_array_nelem(arr);
  for (int i = 0; i < size; i++) {
    if (toml_array_is_primitive(arr)) {
      const toml_raw_t raw = toml_raw_at(arr, i);
      assert(raw != NULL);
      toml_print_raw_value(ts, raw);

      if (i < size - 1) ts_puts(ts, ", ");
    } else if (requiresNewlines) {
      if (i == 0) ts_putc(ts, '\n');

      toml_print_indent(ts, depth + 1);
      toml_print_table_value_inline(ts, toml_table_at(arr, i));
      ts_puts(ts, ",\n");
    } else {
      toml_print_table_value_inline(ts, toml_table_at(arr, i));

      if (i < size - 1) ts_puts(ts, ", ");
    }
  }

  if (requiresNewlines) toml_print_indent(ts, depth);
  ts_puts(ts, "]\n");
}

/// @brief Concatenates two strings, separating them with a period character.
/// The resulting string is allocated on the heap and must be freed by the
/// caller. If the first string is NULL, the second string is duplicated and
/// returned.
/// @param a The first string.
/// @param b The second string.
/// @return The concatenated string.
static char* toml_concat_path(const char* a, const char* b) {
  if (a == NULL) return strdup(b);
  char* c = calloc(strlen(a) + strlen(b) + 2, 1);
  assert(c != NULL);
  strcpy(strcat(strcpy(c, a), "."), b);
  return c;
}

/// @brief Prints a TOML table block to the stream, recursively printing all
/// key-value pairs within the table. The table name is printed as a header
/// followed by a newline suffix. All lines within the table are indented.
/// @param ts The TOML stream to print to.
/// @param depth The current indent depth.
/// @param name The table name.
/// @param tab The TOML table to print.
static void toml_print_table(const TOML* ts, int depth, const char* name,
                             const toml_table_t* tab);

/// @brief Prints a TOML array block to the stream, recursively printing all
/// elements within the array. The array name is printed as a header followed
/// by a newline suffix. All lines within the array are indented.
/// @param ts The TOML stream to print to.
/// @param depth The current indent depth.
/// @param name The array name.
/// @param arr The TOML array to print.
static void toml_print_array(const TOML* ts, int depth, const char* name,
                             const toml_array_t* arr);

static void toml_print_array(const TOML* ts, const int depth, const char* name,
                             const toml_array_t* arr) {
  for (int i = 0; i < toml_array_nelem(arr); i++) {
    ts_putc(ts, '\n');
    toml_print_indent(ts, depth);
    toml_print_array_header(ts, name);
    ts_putc(ts, '\n');

    void* ud;
    if ((ud = (void*) toml_raw_at(arr, i)) != NULL) {
      toml_print_raw_assignment(ts, depth, name, ud);
    } else if ((ud = (void*) toml_table_at(arr, i)) != NULL) {
      toml_print_table(ts, depth, name, ud);
    } else {
      assert("unknown array value type");
    }
  }
}

static void toml_print_table(const TOML* ts, const int depth, const char* name,
                             const toml_table_t* tab) {
  for (int i = 0;; i++) {
    const char* k = toml_key_in(tab, i);
    if (k == NULL) break;

    void* ud;
    if ((ud = (void*) toml_raw_in(tab, k)) != NULL) {
      toml_print_raw_assignment(ts, depth, k, ud);
    } else if ((ud = (void*) toml_array_in(tab, k)) != NULL) {
      if (toml_array_can_be_compacted(ud))
        toml_print_array_assignment(ts, depth, k, ud);
    } else if ((ud = (void*) toml_table_in(tab, k)) != NULL) {
      if (toml_table_can_be_compacted(ud, false))
        toml_print_table_assignment(ts, depth, k, ud);
    }
  }

  for (int i = 0;; i++) {
    const char* k = toml_key_in(tab, i);
    if (k == NULL) break;

    // already printed by initial iteration
    if (toml_raw_in(tab, k) != NULL) continue;

    char* child = toml_concat_path(name, k);
    assert(child != NULL);

    void* ud;
    if ((ud = (void*) toml_array_in(tab, k)) != NULL) {
      if (toml_array_can_be_compacted(ud)) continue;

      toml_print_array(ts, depth + 1, child, ud);
    } else if ((ud = (void*) toml_table_in(tab, k)) != NULL) {
      if (toml_table_can_be_compacted(ud, false)) continue;

      ts_putc(ts, '\n');
      toml_print_indent(ts, depth + 1);
      toml_print_table_header(ts, child);
      ts_putc(ts, '\n');
      toml_print_table(ts, depth + 1, child, ud);
    } else {
      assert("unknown table value type");
    }

    free(child);
  }
}

void toml_print_table_recursive(const TOML* ts, const toml_table_t* tab) {
  toml_print_table(ts, -1, NULL, tab);
}
