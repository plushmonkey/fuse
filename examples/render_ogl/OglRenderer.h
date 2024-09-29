#pragma once

#include <glad/wgl.h>
//
#include <fuse/Fuse.h>

struct OglRenderer {
  void CreateContext();
  void DestroyContext();
  void Render();

  HGLRC hgl = nullptr;
  HWND hwnd = nullptr;
  HDC hdc = nullptr;

  static OglRenderer& Get();

 private:
  OglRenderer() {}
};
