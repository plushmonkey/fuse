#include "SpriteRenderer.h"

#include <rogl/Platform.h>

namespace rogl {

inline void AssertNoErrors(const char* file, int line) {
  GLenum err = glGetError();

  if (err != GL_NO_ERROR) {
    Fatal("GL ERROR [{}:{}]: {}", file, line, err);
  }
}

// #define NO_GLERROR() AssertNoErrors(__FILE__, __LINE__)

constexpr GLsizeiptr kPushBufferMaxCount = 1024;

const char kSpriteVertexShaderCode[] =
    "#version 150"
    R"(
in vec3 position;
in vec2 uv;

uniform mat4 mvp;

out vec2 varying_uv;

void main() {
  gl_Position = mvp * vec4(position, 1.0);
  varying_uv = uv;
}
)";

const char kSpriteFragmentShaderCode[] =
    "#version 150"
    R"(
precision mediump float;

in vec2 varying_uv;

uniform sampler2D color_sampler;

out vec4 color;

void main() {
  color = texture(color_sampler, varying_uv);

  if (color.r == 0.0 && color.g == 0.0 && color.b == 0.0) {
    discard;
  }
}
)";

bool SpriteRenderer::Initialize(int width, int height) {
  if (!shader.Initialize(kSpriteVertexShaderCode, kSpriteFragmentShaderCode)) {
    return false;
  }

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  GLsizeiptr vbo_size = kPushBufferMaxCount * sizeof(SpritePushElement);

  push_buffer = (SpritePushElement*)malloc(vbo_size);

  glBufferData(GL_ARRAY_BUFFER, vbo_size, nullptr, GL_DYNAMIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), 0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (void*)offsetof(SpriteVertex, uv));
  glEnableVertexAttribArray(1);

  mvp_uniform = glGetUniformLocation(shader.program, "mvp");
  color_uniform = glGetUniformLocation(shader.program, "color_sampler");

  shader.Use();
  glUniform1i(color_uniform, 0);

  return true;
}

void SpriteRenderer::Destroy() {
  if (shader.program == -1) return;

  shader.Cleanup();

  push_buffer_count = 0;

  if (push_buffer) {
    free(push_buffer);
    push_buffer = nullptr;
  }

  if (vao != -1) {
    glDeleteVertexArrays(1, &vao);
    vao = -1;
  }

  if (vbo != -1) {
    glDeleteBuffers(1, &vbo);
    vbo = -1;
  }
}

void SpriteRenderer::DrawQuad(const Camera& camera, const Vector2f& dest_dim, const Vector2f& uv_start,
                              const Vector2f& uv_end, GLuint tex_id) {
  SpritePushElement element;

  element.texture = tex_id;
  element.vertices[0].position = Vector3f(0, 0, 0);
  element.vertices[0].uv = Vector2f(uv_start.x, uv_start.y);

  element.vertices[1].position = Vector3f(0, dest_dim.y, 0);
  element.vertices[1].uv = Vector2f(uv_start.x, uv_end.y);

  element.vertices[2].position = Vector3f(dest_dim.x, 0, 0);
  element.vertices[2].uv = Vector2f(uv_end.x, uv_start.y);

  element.vertices[3].position = Vector3f(dest_dim.x, 0, 0);
  element.vertices[3].uv = Vector2f(uv_end.x, uv_start.y);

  element.vertices[4].position = Vector3f(0, dest_dim.y, 0);
  element.vertices[4].uv = Vector2f(uv_start.x, uv_end.y);

  element.vertices[5].position = Vector3f(dest_dim.x, dest_dim.y, 0);
  element.vertices[5].uv = Vector2f(uv_end.x, uv_end.y);

  mat4 proj = camera.GetProjection();
  mat4 view = camera.GetView();
  mat4 mvp = proj * view;

  glUniformMatrix4fv(mvp_uniform, 1, GL_FALSE, (const GLfloat*)mvp.data);
  glBindTexture(GL_TEXTURE_2D, tex_id);
  glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * sizeof(SpriteVertex), &element);

  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void SpriteRenderer::ClearTarget(OglDirectDrawSurface& dest, Rectangle2f* dest_rect) {
  if (dest.flags & SurfaceFlag_Primary) return;

  Vector2f dest_dim((float)dest.desc.dwWidth, (float)dest.desc.dwHeight);

  Vector3f position(0, 0, 0);

  if (dest_rect) {
    position = Vector3f(dest_rect->min, 0);
    dest_dim = dest_rect->GetExtent();
  }

  shader.Use();

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glActiveTexture(GL_TEXTURE0);

  if (dest.fbo == -1) {
    OglRenderer::Get().CreateFramebuffer(dest, true);
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, dest.fbo);

  glScissor((int)position.x, (int)position.y, (int)dest_dim.x, (int)dest_dim.y);
  glViewport((int)position.x, (int)position.y, (int)dest_dim.x, (int)dest_dim.y);

  glEnable(GL_SCISSOR_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glScissor(0, 0, OglRenderer::Get().surface_width, OglRenderer::Get().surface_height);
  glViewport(0, 0, OglRenderer::Get().surface_width, OglRenderer::Get().surface_height);
}

void SpriteRenderer::RenderToTarget(OglDirectDrawSurface& dest, Rectangle2f* dest_rect, const OglDirectDrawSurface& src,
                                    Rectangle2f* src_rect, bool clear_color) {
  if (dest.flags & SurfaceFlag_Primary) {
    Present(dest, src);
    return;
  }

  Vector2f dest_dim((float)dest.desc.dwWidth, (float)dest.desc.dwHeight);
  Vector3f position(0, 0, 0);

  if (dest_rect) {
    position = Vector3f(dest_rect->min, 0);
    dest_dim = dest_rect->GetExtent();
  }

  Camera camera(dest_dim, Vector2f(0, 0), 1.0f);

  Vector2f uv_start(0, 0);
  Vector2f uv_end(1, 1);

  if (src_rect) {
    uv_start.x = (src_rect->min.x / (float)src.desc.dwWidth);
    uv_start.y = (src_rect->max.y / (float)src.desc.dwHeight);

    uv_end.x = (src_rect->max.x / (float)src.desc.dwWidth);
    uv_end.y = (src_rect->min.y / (float)src.desc.dwHeight);
  }

  shader.Use();

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glActiveTexture(GL_TEXTURE0);

  if (dest.fbo == -1) {
    OglRenderer::Get().CreateFramebuffer(dest, true);
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, dest.fbo);

  glScissor((int)position.x, (int)position.y, (int)dest_dim.x, (int)dest_dim.y);
  glViewport((int)position.x, (int)position.y, (int)dest_dim.x, (int)dest_dim.y);

  if (clear_color) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  DrawQuad(camera, dest_dim, uv_start, uv_end, src.tex_id);

  glScissor(0, 0, OglRenderer::Get().surface_width, OglRenderer::Get().surface_height);
  glViewport(0, 0, OglRenderer::Get().surface_width, OglRenderer::Get().surface_height);
}

void SpriteRenderer::Present(const OglDirectDrawSurface& primary, const OglDirectDrawSurface& surface) {
  int width = surface.desc.dwWidth;
  int height = surface.desc.dwHeight;

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glViewport(0, 0, width, height);
  glScissor(0, 0, width, height);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  Vector2f dest_dim((float)surface.desc.dwWidth, (float)surface.desc.dwHeight);
  Vector2f uv_start(0, 0);
  Vector2f uv_end(1, 1);

  Camera camera(dest_dim, Vector2f(0, 0), 1.0f);

  shader.Use();

  glBindVertexArray(vao);

  glActiveTexture(GL_TEXTURE0);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  // Log(LogLevel::Info, "Presenting from {}", surface.tex_id);

  DrawQuad(camera, dest_dim, uv_start, uv_end, surface.tex_id);

  SwapBuffers(OglRenderer::Get().hdc);
}

}  // namespace rogl
