/// @file tomlprint.h
/// @brief High-level API for printing formatted TOML structures to a stream.
#ifndef TOMLREDUCE_TOMLREDUCE_H
#define TOMLREDUCE_TOMLREDUCE_H

#include <tomlc99/toml.h>

/// @struct TOML
/// @brief An interface for writing formatted strings, static strings, and
/// characters to an output destination.
typedef struct toml_stream_s {
  /// @brief Optional user data to pass to the write functions.
  void* udata;
  /// @brief Writes a formatted string to the stream.
  /// @param udata The user data stored in the TOML struct for callback context.
  /// @param fmt The format string.
  /// @param ... The format arguments.
  void (*printf)(void* udata, const char* fmt, ...)
          __attribute__((format(printf, 2, 3)));
  /// @brief Writes a string to the stream.
  /// @param udata The user data stored in the TOML struct for callback context.
  /// @param s The string to write.
  void (*puts)(void* udata, const char* s);
  /// @brief Writes a single character to the stream.
  /// @param udata The user data stored in the TOML struct for callback context.
  /// @param c The character to write.
  void (*putc)(void* udata, char c);
} TOML;

/// @brief Recursively prints a formatted TOML table, including all nested
/// tables and arrays to the given stream.
/// @param ts The TOML stream to write to.
/// @param tab The TOML table to print.
void toml_print_table_recursive(const TOML* ts, const toml_table_t* tab);

#endif//TOMLREDUCE_TOMLREDUCE_H
