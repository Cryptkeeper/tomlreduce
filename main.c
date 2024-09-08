/// @file main.c
/// @brief The main entry point for the program.
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <tomlc99/toml.h>

#include "filestream.h"
#include "tomlprint.h"

/// @brief Reads a TOML file into a TOML table.
/// @param fp The file path to read.
/// @return The TOML table or NULL if the file could not be read or parsed.
static toml_table_t* read_source_file(const char* fp) {
  FILE* src = fopen(fp, "r");
  if (src == NULL) {
    fprintf(stderr, "unable to open file: %s\n", fp);
    return NULL;
  }
  char errmsg[512] = {0};
  toml_table_t* tab = toml_parse_file(src, errmsg, sizeof(errmsg));
  if (tab == NULL) fprintf(stderr, "unable to parse file: %s\n", errmsg);
  fclose(src);
  return tab;
}

int main(const int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <file.toml>\n", argv[0]);
    return 0;
  }

  toml_table_t* tab = read_source_file(argv[1]);
  if (tab == NULL) return 1;

  TOML ts = {0};
  if (toml_open_file_stream(argv[1], &ts)) {
    toml_print_table_recursive(&ts, tab);
    toml_close_file_stream(&ts);
    toml_free(tab);
    return 0;
  } else {
    toml_free(tab);
    fprintf(stderr, "unable to open file stream: %s\n", argv[1]);
    return 1;
  }
}
