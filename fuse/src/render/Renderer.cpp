#include <fuse/render/Renderer.h>
//
#include <ddraw.h>
#include <fuse/Fuse.h>

#pragma comment(lib, "ddraw.lib")
#pragma comment(lib, "dxguid.lib")

namespace fuse {
namespace render {

void Renderer::Reset() {
  renderable_texts.clear();
  renderable_lines.clear();
}

void Renderer::Render() {
  if (!injected) return;

  u32 graphics_addr = *(u32*)(0x4C1AFC) + 0x30;

  if (!graphics_addr) return;

  LPDIRECTDRAWSURFACE back_surface = (LPDIRECTDRAWSURFACE) * (u32*)(graphics_addr + 0x44);

  if (!back_surface) return;

  HDC hdc;
  back_surface->GetDC(&hdc);

  HGDIOBJ obj = SelectObject(hdc, GetStockObject(DC_PEN));

  for (RenderableLine& renderable : renderable_lines) {
    SetDCPenColor(hdc, renderable.color.value);
    MoveToEx(hdc, (int)renderable.from.x, (int)renderable.from.y, NULL);
    LineTo(hdc, (int)renderable.to.x, (int)renderable.to.y);
  }

  back_surface->ReleaseDC(hdc);

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

void Renderer::PushText(std::string_view text, const Vector2f& position, TextColor color, RenderTextFlags flags) {
  if (!injected) return;

  RenderableText renderable;

  renderable.text = text;
  renderable.position = position;
  renderable.color = color;
  renderable.flags = flags;

  renderable_texts.push_back(std::move(renderable));
}

void Renderer::PushWorldLine(const Vector2f& world_from, const Vector2f& world_to, Color color) {
  auto player = Fuse::Get().GetPlayer();

  if (!player) return;

  Vector2f surface_center = GetSurfaceSize() * 0.5f;
  Vector2f world_center_position = player->position;

  Vector2f diff = world_to - world_from;
  Vector2f from = (world_from - world_center_position) * 16.0f;
  Vector2f to = from + (diff * 16.0f);

  Vector2f start = surface_center + from;
  Vector2f end = surface_center + to;

  PushScreenLine(start, end, color);
}

void Renderer::PushScreenLine(const Vector2f& screen_from, const Vector2f& screen_to, Color color) {
  // TODO: Check if in screen
  RenderableLine renderable;

  renderable.from = screen_from;
  renderable.to = screen_to;
  renderable.color = color;

  renderable_lines.push_back(renderable);
}

Vector2f Renderer::GetSurfaceSize() const {
  HWND hWnd = Fuse::Get().GetGameWindowHandle();

  if (!hWnd) {
    return Vector2f(0, 0);
  }

  RECT rect;
  GetClientRect(hWnd, &rect);

  return Vector2f((float)rect.right, (float)rect.bottom);
}

}  // namespace render
}  // namespace fuse
