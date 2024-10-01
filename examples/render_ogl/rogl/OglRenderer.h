#pragma once

#include <glad/wgl.h>
//
#include <fuse/Fuse.h>

#include <memory>
#include <vector>

struct OglDirectDrawSurface;

namespace rogl {

struct SpriteRenderer;

struct OglRenderer {
  void CreateContext();
  void DestroyContext();

  GLuint CreateTexture();
  void CreateFramebuffer(OglDirectDrawSurface& surface, bool upload = false);

  void UploadTexture(OglDirectDrawSurface& surface, fuse::u8* data, size_t size);

  HGLRC hgl = nullptr;
  HWND hwnd = nullptr;
  HDC hdc = nullptr;

  int surface_width = 0;
  int surface_height = 0;

  std::unique_ptr<SpriteRenderer> sprite_renderer;

  std::vector<GLuint> textures_allocated;

  static OglRenderer& Get();

 private:
  OglRenderer();
};

}  // namespace rogl
