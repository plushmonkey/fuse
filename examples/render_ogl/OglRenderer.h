#pragma once

#include <glad/wgl.h>
//
#include <fuse/Fuse.h>

struct OglRenderer {
  void CreateContext();
  void DestroyContext();

  HGLRC hgl = nullptr;
  HWND hwnd = nullptr;
  HDC hdc = nullptr;
};
