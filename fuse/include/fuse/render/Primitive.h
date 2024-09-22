#pragma once

#include <fuse/Math.h>
#include <fuse/render/Color.h>

namespace fuse {
namespace render {

struct RenderableLine {
  Vector2f from;
  Vector2f to;
  Color color;
};

struct RenderableQuad {
  Vector2f position;
  Vector2f extent;
  Color color;
};

}  // namespace render
}  // namespace fuse
