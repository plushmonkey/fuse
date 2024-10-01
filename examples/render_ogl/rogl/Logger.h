#pragma once

#include <format>
#include <string_view>

namespace rogl {

enum class LogLevel { Jabber, Debug, Info, Warning, Error, Count };

extern LogLevel g_LogPrintLevel;
extern const char* g_LogPath;

void _Log(LogLevel level, std::string_view message);

template <class... _Types>
void Log(LogLevel level, const std::format_string<_Types...> _Fmt, _Types&&... _Args) {
  std::string message = std::vformat(_Fmt.get(), std::make_format_args(_Args...));
  _Log(level, message);
}

}  // namespace rogl
