#pragma once

#include <rogl/Math.h>

namespace rogl {

struct Camera {
  Vector2f position;
  mat4 projection;
  Vector2f surface_dim;
  float scale;

  Camera(const Vector2f& surface_dim, const Vector2f& position, float scale)
      : surface_dim(surface_dim), position(position), scale(scale) {
    float zmax = (float)2.0f;
#if 0
    projection = Orthographic(-surface_dim.x / 2.0f * scale, surface_dim.x / 2.0f * scale, surface_dim.y / 2.0f * scale,
                              -surface_dim.y / 2.0f * scale, -zmax, zmax);
#endif
    projection = Orthographic(0, surface_dim.x * scale, surface_dim.y * scale, 0, -zmax, zmax);
  }

  inline mat4 GetView() const { return Translate(mat4::Identity(), Vector3f(-position.x, -position.y, 0.0f)); }
  inline mat4 GetProjection() const { return projection; }
};

}  // namespace rogl
