/// @file filestream.c
/// @brief Implementation for wrapping a file as a TOML print stream.
#include "filestream.h"

#include <assert.h>
#include <stdarg.h>

static void toml_file_stream_printf(void* udata, const char* fmt, ...) {
  assert(udata != NULL);
  va_list args;
  va_start(args, fmt);
  vfprintf(udata, fmt, args);
  va_end(args);
}

static void toml_file_stream_puts(void* udata, const char* s) {
  assert(udata != NULL);
  fputs(s, udata);
}

static void toml_file_stream_putc(void* udata, char c) {
  assert(udata != NULL);
  fputc(c, udata);
}

bool toml_open_file_stream(const char* fp, TOML* ts) {
  FILE* f = fopen(fp, "w");
  if (f == NULL) return false;
  *ts = (TOML){
          .udata = f,
          .printf = toml_file_stream_printf,
          .puts = toml_file_stream_puts,
          .putc = toml_file_stream_putc,
  };
  return true;
}

void toml_close_file_stream(TOML* ts) {
  if (ts->udata != NULL) fclose(ts->udata);
  ts->udata = NULL;
}
