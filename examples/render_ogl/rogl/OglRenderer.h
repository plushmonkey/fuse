#pragma once

#include <glad/wgl.h>
//
#include <fuse/Fuse.h>

namespace rogl {

// TODO: Track textures given out so they can be cleaned up.
struct OglRenderer {
  void CreateContext();
  void DestroyContext();
  void Render();

  GLuint CreateTexture();
  void UploadTexture(GLuint tex_id, int width, int height, fuse::u8* data, size_t size);

  HGLRC hgl = nullptr;
  HWND hwnd = nullptr;
  HDC hdc = nullptr;

  static OglRenderer& Get();

 private:
  OglRenderer() {}
};

}  // namespace rogl
