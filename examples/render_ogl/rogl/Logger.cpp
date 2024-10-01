#include "Logger.h"

#include <fuse/Types.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

namespace rogl {

LogLevel g_LogPrintLevel = LogLevel::Debug;
const char* g_LogPath = "rogl.log";

void _Log(LogLevel level, std::string_view message) {
  static const char kLevelCharacters[] = {'J', 'D', 'I', 'W', 'E'};

  static_assert(FUSE_ARRAY_SIZE(kLevelCharacters) == (size_t)LogLevel::Count);

  if ((size_t)level < (size_t)g_LogPrintLevel) return;

  time_t now = time(nullptr);
  tm* tm = localtime(&now);

  char level_c = 'D';

  if ((size_t)level < FUSE_ARRAY_SIZE(kLevelCharacters)) {
    level_c = kLevelCharacters[(size_t)level];
  }

  auto console_file = stdout;

  if ((size_t)level >= (size_t)LogLevel::Warning) {
    console_file = stderr;
  }

  fprintf(console_file, "%c [%02d:%02d:%02d] %.*s\n", level_c, tm->tm_hour, tm->tm_min, tm->tm_sec,
          (unsigned int)message.size(), message.data());
  fflush(console_file);

  if (g_LogPath) {
    FILE* f = fopen(g_LogPath, "a");

    if (f) {
      fprintf(f, "%c [%02d:%02d:%02d] %.*s\n", level_c, tm->tm_hour, tm->tm_min, tm->tm_sec,
              (unsigned int)message.size(), message.data());
      fflush(f);
      fclose(f);
    }
  }
}

}  // namespace rogl
