#include <fuse/render/GDIRenderer.h>
//
#include <ddraw.h>
#include <detours.h>
#include <fuse/Fuse.h>

#pragma comment(lib, "ddraw.lib")
#pragma comment(lib, "dxguid.lib")

namespace fuse {
namespace render {

static HRESULT(STDMETHODCALLTYPE* RealBlt)(LPDIRECTDRAWSURFACE, LPRECT, LPDIRECTDRAWSURFACE, LPRECT, DWORD, LPDDBLTFX);
static HRESULT(STDMETHODCALLTYPE* RealFlip)(LPDIRECTDRAWSURFACE, DWORD);

HRESULT STDMETHODCALLTYPE OverrideBlt(LPDIRECTDRAWSURFACE surface, LPRECT dest_rect, LPDIRECTDRAWSURFACE next_surface,
                                      LPRECT src_rect, DWORD flags, LPDDBLTFX fx) {
  u32 graphics_addr = *(u32*)(0x4C1AFC) + 0x30;
  LPDIRECTDRAWSURFACE primary_surface = (LPDIRECTDRAWSURFACE) * (u32*)(graphics_addr + 0x40);
  LPDIRECTDRAWSURFACE back_surface = (LPDIRECTDRAWSURFACE) * (u32*)(graphics_addr + 0x44);

  // Check if flipping. I guess there's a full screen blit instead of flip when running without vsync?
  if (surface == primary_surface && next_surface == back_surface && fx == 0) {
    Fuse::Get().GetRenderer().Render();
  } else {
#if 0
    CallAddress caller = Fuse::Get().GetCallAddress();

    // The text texture might be an optimization for turning many text draw calls into one saved texture.
    // if (caller.address == 0x404d83) return S_OK; // Render text texture
    // if (caller.address == 0x4059e2) return S_OK; // Render text texture
    // if (caller.address == 0x405797) return S_OK; // Clear text texture

    // if (caller.address == 0x404c04) return S_OK; // Render sprite/tile
    // if (caller.address == 0x404cc8) return S_OK; // Clipped screen edge sprite/tile
    // if (caller.address == 0x40565e) return S_OK; // Clear background
    
    // if (caller.address == 0x405cdd) return S_OK; // Horizontal window border
    // if (caller.address == 0x405e20) return S_OK; // Vertical window border

    // if (caller.address == 0x402467) return S_OK; // Energy bar

    if (caller.address == 0x404c04 || caller.address == 0x404cc8) {
      struct ContinuumTexture {
        u32 vtable;
        u32 unknown1;
        u32 surface_ptr;
        u32 unknown2;
        u32 unknown3;
        u32 unknown4;
        u32 unknown5;
        u32 unknown6;
        u32 unknown7;
        u32 width;
        u32 height;
        char name[544];
      };

      ContinuumTexture* map_texture = *(ContinuumTexture**)(*(u32*)(0x4c1afc) + 0x281C);

      // Filter out tile rendering
      if (map_texture->surface_ptr == (u32)next_surface) return S_OK;
    }
#endif
  }

  return RealBlt(surface, dest_rect, next_surface, src_rect, flags, fx);
}

HRESULT STDMETHODCALLTYPE OverrideFlip(LPDIRECTDRAWSURFACE surface, DWORD flags) {
  u32 graphics_addr = *(u32*)(0x4C1AFC) + 0x30;
  LPDIRECTDRAWSURFACE primary_surface = (LPDIRECTDRAWSURFACE) * (u32*)(graphics_addr + 0x40);
  LPDIRECTDRAWSURFACE back_surface = (LPDIRECTDRAWSURFACE) * (u32*)(graphics_addr + 0x44);

  Fuse::Get().GetRenderer().Render();

  return RealFlip(surface, flags);
}

void GDIRenderer::OnNewFrame() {
  renderable_texts.clear();
  renderable_lines.clear();
  renderable_quads.clear();

  if (!IsInjected()) {
    Inject();
  }
}

void GDIRenderer::Render() {
  if (!IsInjected()) return;

  u32 graphics_addr = *(u32*)(0x4C1AFC) + 0x30;

  if (!graphics_addr) return;

  LPDIRECTDRAWSURFACE back_surface = (LPDIRECTDRAWSURFACE) * (u32*)(graphics_addr + 0x44);

  if (!back_surface) return;

  HDC hdc;
  back_surface->GetDC(&hdc);

  SelectObject(hdc, GetStockObject(DC_PEN));
  SelectObject(hdc, GetStockObject(DC_BRUSH));

  // Render quads, then lines, then text.
  // It's rendered in this order for most likely desired ordering.

  for (RenderableQuad& renderable : renderable_quads) {
    SetDCPenColor(hdc, renderable.color.value);
    SetDCBrushColor(hdc, renderable.color.value);

    int left = (int)renderable.position.x;
    int top = (int)renderable.position.y;
    int right = left + (int)renderable.extent.x;
    int bottom = top + (int)renderable.extent.y;

    Rectangle(hdc, left, top, right, bottom);
  }

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
    } else if (renderable.flags & RenderText_AlignRight) {
      x -= (u32)(renderable.text.length() * 8.0f);
    }

    render_text(This, 0, x, y, renderable.text.c_str(), (int)renderable.color, -1, 1);
  }
}

void GDIRenderer::PushText(std::string_view text, const Vector2f& position, TextColor color, RenderTextFlags flags) {
  //
  if (!injected) return;

  RenderableText renderable;

  renderable.text = text;
  renderable.position = position;
  renderable.color = color;
  renderable.flags = flags;

  renderable_texts.push_back(std::move(renderable));
}

void GDIRenderer::PushWorldLine(const Vector2f& world_from, const Vector2f& world_to, Color color) {
  auto player = Fuse::Get().GetPlayer();

  if (!player) return;

  Vector2f surface_center = GetSurfaceSize() * 0.5f;
  s32* camera = (s32*)(Fuse::Get().GetGameMemory().game_address + 0x27f8);
  Vector2f world_center_position(camera[0] / 16.0f, camera[1] / 16.0f);

  Vector2f diff = world_to - world_from;
  Vector2f from = (world_from - world_center_position) * 16.0f;
  Vector2f to = from + (diff * 16.0f);

  Vector2f start = surface_center + from;
  Vector2f end = surface_center + to;

  PushScreenLine(start, end, color);
}

void GDIRenderer::PushScreenLine(const Vector2f& screen_from, const Vector2f& screen_to, Color color) {
  // TODO: Check if in screen
  RenderableLine renderable;

  renderable.from = screen_from;
  renderable.to = screen_to;
  renderable.color = color;

  renderable_lines.push_back(renderable);
}

void GDIRenderer::PushScreenQuad(const Vector2f& screen_position, const Vector2f& extent, Color color) {
  // TODO: Check if in screen
  RenderableQuad renderable;

  renderable.position = screen_position;
  renderable.extent = extent;
  renderable.color = color;

  renderable_quads.push_back(renderable);
}

void GDIRenderer::PushScreenBorder(const Vector2f& position, const Vector2f& extent, Color color, float size) {
  float total_width = extent.x + size * 2;
  float total_height = extent.y + size * 2;

  PushScreenQuad(position + Vector2f(-size, -size), Vector2f(total_width, size), color);
  PushScreenQuad(position + Vector2f(-size, -size), Vector2f(size, total_height), color);
  PushScreenQuad(position + Vector2f(extent.x, -size), Vector2f(size, total_height), color);
  PushScreenQuad(position + Vector2f(-size, extent.y), Vector2f(total_width, size), color);
}

Vector2f GDIRenderer::GetSurfaceSize() const {
  HWND hWnd = Fuse::Get().GetGameWindowHandle();

  if (!hWnd) {
    return Vector2f(0, 0);
  }

  RECT rect;
  GetClientRect(hWnd, &rect);

  return Vector2f((float)rect.right, (float)rect.bottom);
}

void GDIRenderer::Inject() {
  if (IsInjected()) return;

  u32 graphics_addr = *(u32*)(0x4C1AFC) + 0x30;
  if (graphics_addr) {
    LPDIRECTDRAWSURFACE surface = (LPDIRECTDRAWSURFACE) * (u32*)(graphics_addr + 0x44);

    if (surface) {
      void** vtable = (*(void***)surface);
      RealBlt = (HRESULT(STDMETHODCALLTYPE*)(LPDIRECTDRAWSURFACE surface, LPRECT, LPDIRECTDRAWSURFACE, LPRECT, DWORD,
                                             LPDDBLTFX))vtable[5];
      RealFlip = (HRESULT(STDMETHODCALLTYPE*)(LPDIRECTDRAWSURFACE surface, DWORD))vtable[11];

      DetourRestoreAfterWith();

      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());
      DetourAttach(&(PVOID&)RealBlt, OverrideBlt);
      DetourAttach(&(PVOID&)RealFlip, OverrideFlip);
      DetourTransactionCommit();

      this->injected = true;
    }
  }
}

}  // namespace render
}  // namespace fuse
