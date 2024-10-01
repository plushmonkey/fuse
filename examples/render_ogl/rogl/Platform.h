#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <rogl/Logger.h>

#include <format>
#include <string_view>

namespace rogl {

template <class... _Types>
void DisplayMessage(const std::format_string<_Types...> _Fmt, _Types&&... _Args) {
  std::string message = std::vformat(_Fmt.get(), std::make_format_args(_Args...));
  MessageBox(NULL, message.data(), "ogl", MB_OK | MB_ICONERROR);
}

template <class... _Types>
void Fatal(const std::format_string<_Types...> _Fmt, _Types&&... _Args) {
  std::string message = std::vformat(_Fmt.get(), std::make_format_args(_Args...));
  MessageBox(NULL, message.data(), "ogl", MB_OK | MB_ICONERROR);
  exit(1);
}

}  // namespace rogl
