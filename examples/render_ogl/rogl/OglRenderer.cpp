#include "OglRenderer.h"

#include <fuse/Fuse.h>
#include <rogl/Platform.h>

#include <format>
#include <string>
#include <string_view>

using namespace fuse;

namespace rogl {

OglRenderer& OglRenderer::Get() {
  static OglRenderer instance;
  return instance;
}

void OglRenderer::Render() {
  if (!this->hgl) return;

  glViewport(0, 0, 1360, 768);
  glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  SwapBuffers(hdc);
}

GLuint OglRenderer::CreateTexture() {
  GLuint tex_id = -1;

  glGenTextures(1, &tex_id);
  glBindTexture(GL_TEXTURE_2D, tex_id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  return tex_id;
}

void OglRenderer::UploadTexture(GLuint tex_id, int width, int height, u8* data, size_t size) {
  glBindTexture(GL_TEXTURE_2D, tex_id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void OglRenderer::CreateContext() {
  this->hwnd = Fuse::Get().GetGameWindowHandle();
  this->hdc = GetDC(hwnd);

  // clang-format off
  PIXELFORMATDESCRIPTOR pfd =
  {
    sizeof(PIXELFORMATDESCRIPTOR),
    1,
    PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    // Flags
    PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
    32,                   // Colordepth of the framebuffer.
    0, 0, 0, 0, 0, 0,
    0,
    0,
    0,
    0, 0, 0, 0,
    24,                   // Number of bits for the depthbuffer
    8,                    // Number of bits for the stencilbuffer
    0,                    // Number of Aux buffers in the framebuffer.
    PFD_MAIN_PLANE,
    0,
    0, 0, 0
  };
  // clang-format on

  int pfd_idx = ChoosePixelFormat(hdc, &pfd);
  if (pfd_idx == 0) {
    Fatal("Failed to choose pixel format for context bootstrapping.");
  }

  if (!SetPixelFormat(hdc, pfd_idx, &pfd)) {
    Fatal("Failed to SetPixelFormat for context bootstrapping.");
  }

  HGLRC temp_context = wglCreateContext(hdc);

  if (!wglMakeCurrent(hdc, temp_context)) {
    Fatal("Failed to wglMakeCurrent for temp context.");
  }

  gladLoaderLoadWGL(hdc);

  // clang-format off
  int attributes[] = {
    WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
    WGL_CONTEXT_MINOR_VERSION_ARB, 2,
    WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
    0
  };
  // clang-format on

  hgl = wglCreateContextAttribsARB(hdc, NULL, attributes);

  if (!hgl) {
    wglDeleteContext(temp_context);
    Fatal("Failed to create OpenGL context");
  }

  wglMakeCurrent(NULL, NULL);
  wglDeleteContext(temp_context);
  wglMakeCurrent(hdc, hgl);

  if (!gladLoaderLoadGL()) {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hgl);
    Fatal("Glad loader failed.");
  }
}

void OglRenderer::DestroyContext() {
  if (!hdc) return;

  if (hgl) {
    wglMakeCurrent(hdc, nullptr);
    wglDeleteContext(hgl);
  }

  hgl = nullptr;
  // This window owns the dc, so don't release it.
  hdc = nullptr;
  hwnd = nullptr;
}

}  // namespace rogl
