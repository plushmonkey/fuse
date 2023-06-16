#pragma once

#include <fuse/render/Color.h>
#include <fuse/render/Text.h>

#include <vector>

namespace fuse {
namespace render {

struct RenderableLine {
  Vector2f from;
  Vector2f to;
  Color color;
};

struct Renderer {
  bool injected = false;

  FUSE_EXPORT void Reset();
  FUSE_EXPORT void Render();

  FUSE_EXPORT void PushText(std::string_view text, const Vector2f& position, TextColor color, RenderTextFlags flags = 0);
  FUSE_EXPORT void PushWorldLine(const Vector2f& world_from, const Vector2f& world_to, Color color);
  FUSE_EXPORT void PushScreenLine(const Vector2f& screen_from, const Vector2f& screen_to, Color color);

  FUSE_EXPORT Vector2f GetSurfaceSize() const;

private:
  std::vector<RenderableText> renderable_texts;
  std::vector<RenderableLine> renderable_lines;

};

}  // namespace render
}  // namespace fuse
