/// @file filestream.h
/// @brief API for wrapping a file as a TOML print stream.
#ifndef TOMLREDUCE_FILESTREAM_H
#define TOMLREDUCE_FILESTREAM_H

#include <stdbool.h>

#include "tomlprint.h"

/// @brief Opens a file stream for writing and wraps it in a TOML stream.
/// @param fp The file path to open.
/// @param ts The TOML stream to initialize. If opened, the stream must be be
/// closed with \p toml_close_file_stream.
/// @return true if the file was opened successfully and \p ts was populated,
/// false otherwise and \p ts is not modified.
bool toml_open_file_stream(const char* fp, TOML* ts);

/// @brief Closes a file stream if open.
/// @param ts The TOML stream to close, must not be NULL.
void toml_close_file_stream(TOML* ts);

#endif//TOMLREDUCE_FILESTREAM_H
