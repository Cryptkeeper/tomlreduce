/// @file fmtlogic.h
/// @brief Utility logic implementation for formatting TOML structures.
#ifndef TOMLREDUCE_FMTLOGIC_H
#define TOMLREDUCE_FMTLOGIC_H

#include <stdbool.h>

#include <tomlc99/toml.h>

/// @brief Determines if a table can be compacted into a single line, or if it
/// should be printed in an expanded square bracket form.
/// @param tab The table to check.
/// @param checkStringLength If true, check the length of strings in the table
/// for wrapping, otherwise string value length is ignored.
/// @return true if the table can be compacted, false otherwise.
bool toml_table_can_be_compacted(const toml_table_t* tab,
                                 bool checkStringLength);

/// @brief Determines if an array can be compacted into its curly bracket form,
/// or if it should be printed in an expanded square bracket form.
/// @param arr The array to check.
/// @return true if the array can be compacted, false otherwise.
bool toml_array_can_be_compacted(const toml_array_t* arr);

/// @brief Determines if an array should use a newline between elements, or if
/// the entire array can be printed on a single line.
/// @param arr The array to check.
/// @return true if the array should use a newline between elements, false
/// otherwise.
bool toml_array_should_use_newline_between_elements(const toml_array_t* arr);

/// @brief Determines if an array contains only primitive values.
/// @param arr The array to check.
/// @return true if the array contains only primitive values, false otherwise.
bool toml_array_is_primitive(const toml_array_t* arr);

#endif//TOMLREDUCE_FMTLOGIC_H
