#include "OglRenderer.h"

#include <fuse/Fuse.h>
#include <rogl/Platform.h>
#include <rogl/SpriteRenderer.h>
#include <rogl/ddraw/IDirectDrawSurface.h>

#include <format>
#include <string>
#include <string_view>

using namespace fuse;

namespace rogl {

OglRenderer::OglRenderer() {
  textures_allocated.reserve(64);
  sprite_renderer = std::make_unique<SpriteRenderer>();
}

OglRenderer& OglRenderer::Get() {
  static OglRenderer instance;
  return instance;
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

  textures_allocated.push_back(tex_id);

  return tex_id;
}

void OglRenderer::CreateFramebuffer(OglDirectDrawSurface& surface, bool upload) {
  GLuint framebuffer;

  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  glBindTexture(GL_TEXTURE_2D, surface.tex_id);

  if (upload) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, surface.desc.dwWidth, surface.desc.dwHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 0);
  }

  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, surface.tex_id, 0);

  GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, draw_buffers);

  GLenum frame_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (frame_status != GL_FRAMEBUFFER_COMPLETE) {
    Fatal("Bad frame status during creation: {}", frame_status);
  }

  surface.fbo = framebuffer;
}

void OglRenderer::UploadTexture(OglDirectDrawSurface& surface, u8* data, size_t size) {
  GLuint tex_id = surface.tex_id;
  int width = surface.desc.dwWidth;
  int height = surface.desc.dwHeight;

  glBindTexture(GL_TEXTURE_2D, tex_id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  CreateFramebuffer(surface);
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

  RECT rect = {};
  GetClientRect(this->hwnd, &rect);

  surface_width = rect.right - rect.left;
  surface_height = rect.bottom - rect.top;

  if (!sprite_renderer->Initialize(surface_width, surface_height)) {
    Fatal("Failed to initialize sprite renderer.");
  }

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_SCISSOR_TEST);
  wglSwapIntervalEXT(0);
}

void OglRenderer::DestroyContext() {
  if (!hdc) return;

  if (hgl) {
    sprite_renderer->Destroy();

    if (!textures_allocated.empty()) {
      glDeleteTextures(textures_allocated.size(), textures_allocated.data());
      textures_allocated.clear();
    }

    wglMakeCurrent(hdc, nullptr);
    wglDeleteContext(hgl);
  }

  hgl = nullptr;
  // This window owns the dc, so don't release it.
  hdc = nullptr;
  hwnd = nullptr;
}

}  // namespace rogl
