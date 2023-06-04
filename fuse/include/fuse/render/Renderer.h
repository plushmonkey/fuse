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
  std::vector<RenderableText> renderable_texts;
  std::vector<RenderableLine> renderable_lines;

  bool injected = false;

  void Reset();
  void Render();

  void PushText(std::string_view text, const Vector2f& position, TextColor color, RenderTextFlags flags = 0);
  void PushWorldLine(const Vector2f& world_from, const Vector2f& world_to, Color color);
  void PushScreenLine(const Vector2f& screen_from, const Vector2f& screen_to, Color color);

  Vector2f GetSurfaceSize() const;
};

}  // namespace render
}  // namespace fuse
