#pragma once

#include <fuse/render/Text.h>

#include <vector>

namespace fuse {
namespace render {

struct Renderer {
  std::vector<RenderableText> renderable_texts;

  bool injected = false;

  void Reset();
  void Render();
  void RenderText(std::string_view text, const Vector2f& position, TextColor color, RenderTextFlags flags = 0);
};

}  // namespace render
}  // namespace fuse
