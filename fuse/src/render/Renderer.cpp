#include <fuse/render/Renderer.h>
//
#include <ddraw.h>

#pragma comment(lib, "ddraw.lib")
#pragma comment(lib, "dxguid.lib")

namespace fuse {
namespace render {

void Renderer::Reset() {
  renderable_texts.clear();
}

void Renderer::Render() {
  if (!injected) return;

  u32 graphics_addr = *(u32*)(0x4C1AFC) + 0x30;
  LPDIRECTDRAWSURFACE back_surface = (LPDIRECTDRAWSURFACE) * (u32*)(graphics_addr + 0x44);

  typedef void(__fastcall * RenderTextFunc)(void* This, void* thiscall_garbage, int x, int y, const char* text,
                                            int zero, int length, u8 alpha);

  RenderTextFunc render_text = (RenderTextFunc)(0x442FE0);
  void* This = (void*)(graphics_addr);

  for (RenderableText& renderable : renderable_texts) {
    u32 x = (u32)renderable.position.x;
    u32 y = (u32)renderable.position.y;

    if (renderable.flags & RenderText_Centered) {
      x -= (u32)((renderable.text.length() / 2.0f) * 8.0f);
    }

    render_text(This, 0, x, y, renderable.text.c_str(), (int)renderable.color, -1, 1);
  }
}

void Renderer::RenderText(std::string_view text, const Vector2f& position, TextColor color, RenderTextFlags flags) {
  if (!injected) return;

  RenderableText renderable;

  renderable.text = text;
  renderable.position = position;
  renderable.color = color;
  renderable.flags = flags;

  renderable_texts.push_back(std::move(renderable));
}

}  // namespace render
}  // namespace fuse
