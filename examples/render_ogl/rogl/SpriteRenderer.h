#pragma once

#include <rogl/Camera.h>
#include <rogl/Math.h>
#include <rogl/OglRenderer.h>
#include <rogl/Shader.h>
#include <rogl/ddraw/IDirectDrawSurface.h>

#include <vector>

namespace rogl {

struct SpriteVertex {
  Vector3f position;
  Vector2f uv;

  SpriteVertex() {}
  SpriteVertex(const Vector3f& position, const Vector2f& uv) : position(position), uv(uv) {}
};

struct SpritePushElement {
  SpriteVertex vertices[6];
  GLuint texture;
};

struct SpriteRenderer {
  bool Initialize(int width, int height);
  void Destroy();

  void ClearTarget(OglDirectDrawSurface& dest, Rectangle2f* dest_rect);

  void RenderToTarget(OglDirectDrawSurface& dest, Rectangle2f* dest_rect, const OglDirectDrawSurface& src,
                      Rectangle2f* src_rect, bool clear_color);
  void Present(const OglDirectDrawSurface& primary, const OglDirectDrawSurface& surface);

  size_t push_buffer_count = 0;
  SpritePushElement* push_buffer = nullptr;

  GLuint vao = -1;
  GLuint vbo = -1;

  GLint color_uniform = -1;
  GLint mvp_uniform = -1;

  Shader shader;

 private:
  void DrawQuad(const Camera& camera, const Vector2f& dest_dim, const Vector2f& uv_start, const Vector2f& uv_end,
                GLuint tex_id);
};

}  // namespace rogl
