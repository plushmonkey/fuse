#include <fuse/Fuse.h>
#include <fuse/HookInjection.h>

using namespace fuse;

#include <format>
#include <fstream>

struct Painter {
  struct LineSegment {
    Vector2i start;
    Vector2i end;

    LineSegment() {}
    LineSegment(Vector2i start, Vector2i end) : start(start), end(end) {}
  };

  void Render(render::Color color) {
    if (Fuse::Get().IsGameMenuOpen()) {
      lines.clear();
    }

    for (LineSegment& segment : lines) {
      Fuse::Get().GetRenderer().PushScreenLine(segment.start.ToVector2f(), segment.end.ToVector2f(), color);
    }
  }

  void Clear() {
    lines.clear();
    painting = false;
  }

  void OnMouseUp(const Vector2i& position, MouseButton button) {
    if (button == MouseButton::Left) {
      lines.emplace_back(position, position);
    } else if (button == MouseButton::Middle) {
      lines.clear();
    }

    painting = false;
  }

  void OnMouseMove(const Vector2i& position, MouseButtons buttons) {
    if (!buttons.IsDown(MouseButton::Left)) return;

    if (lines.empty() || !painting) {
      lines.emplace_back(position, position);
      painting = true;
      return;
    }

    if (lines.back().end == position) return;

    Vector2f new_p = position.ToVector2f();
    Vector2f old_p = lines.back().end.ToVector2f();

    lines.emplace_back(old_p, new_p);
  }

 private:
  std::vector<LineSegment> lines;
  bool painting = false;
};

class HelloWorld final : public HookInjection {
 public:
  const char* GetHookName() override { return "HelloWorld"; }

  void OnUpdate() override {
    std::string output = "Hello, world.";

    // Calculate a hue that cycles based on the current tick.
    float hue = ((GetTickCount() % 3000) / 3000.0f) * 360.0f;
    render::Color color = render::Color::FromHSV(hue, 0.8f, 0.6f);

    Player* player = Fuse::Get().GetPlayer();
    ConnectState connect_state = Fuse::Get().GetConnectState();

    if (connect_state == ConnectState::Playing) {
      if (player && player->ship != 8) {
        output = std::format("Hello, {} ({}). You are at ({:.2f}, {:.2f}).", player->GetName(), player->id,
                             player->position.x, player->position.y);

        // Check if the player is not in a safe.
        if (Fuse::Get().GetMap().GetTileId(player->position) != kSafeTileId) {
          // Render a line showing the direction the player is facing.
          Fuse::Get().GetRenderer().PushWorldLine(player->position, player->position + player->GetHeading() * 3.0f,
                                                  color);
        }

        y = 244.0f;
        RenderDebugText(std::format("Zone: {}", Fuse::Get().GetZoneName()));
        RenderDebugText(std::format("Arena: {}", Fuse::Get().GetArenaName()));
        RenderDebugText(std::format("Map: {}", Fuse::Get().GetMapName()));

        auto cap = Fuse::Get().GetShipStatus().capability;

#if 0
        RenderDebugText(std::format("Can stealth: {}", cap.stealth != 0));
        RenderDebugText(std::format("Can cloak: {}", cap.cloak != 0));
        RenderDebugText(std::format("Can antiwarp: {}", cap.antiwarp != 0));
        RenderDebugText(std::format("Can xradar: {}", cap.xradar != 0));
        RenderDebugText(std::format("Can multifire: {}", cap.multifire != 0));
        RenderDebugText(std::format("Can prox: {}", cap.proximity != 0));
        RenderDebugText(std::format("Can bounce bullets: {}", cap.bouncing_bullets != 0));
#endif
        RenderDebugText(std::format("Guns: {}, Bombs: {}", Fuse::Get().GetShipStatus().guns, Fuse::Get().GetShipStatus().bombs));

        RenderDebugText(std::format("Repels: {}", Fuse::Get().GetShipStatus().repels));
        RenderDebugText(std::format("Bursts: {}", Fuse::Get().GetShipStatus().bursts));
        RenderDebugText(std::format("Bricks: {}", Fuse::Get().GetShipStatus().bricks));
        RenderDebugText(std::format("Rockets: {}", Fuse::Get().GetShipStatus().rockets));
        RenderDebugText(std::format("Thors: {}", Fuse::Get().GetShipStatus().thors));
        RenderDebugText(std::format("Decoys: {}", Fuse::Get().GetShipStatus().decoys));
        RenderDebugText(std::format("Portals: {}", Fuse::Get().GetShipStatus().portals));

        RenderDebugText(std::format("Status: {}", player->status));
      }

      painter.Render(color);
    } else {
      const char* state_str = to_string(connect_state);

      output = std::format("ConnectState: {}", state_str);
      painter.Clear();
    }

    Vector2f surface_center = Fuse::Get().GetRenderer().GetSurfaceSize() * 0.5f;

    float text_x = surface_center.x - 400.0f;
    if (text_x < 0) text_x = 0;

    Fuse::Get().GetRenderer().PushText(output, Vector2f(text_x, 300), render::TextColor::Yellow);

    float line_length = (float)(output.size() * 8);
    Fuse::Get().GetRenderer().PushScreenLine(Vector2f(text_x, 312), Vector2f(text_x + line_length, 312), color);
  }

  inline void RenderDebugText(const std::string& text) {
    Vector2f surface_center = Fuse::Get().GetRenderer().GetSurfaceSize() * 0.5f;

    float text_x = surface_center.x + 100.0f;

    Fuse::Get().GetRenderer().PushText(text, Vector2f(text_x, y), render::TextColor::Pink);
    y += 12.0f;
  }

  const char* to_string(ConnectState connect_state) {
    static const char* kStates[] = {"Menu",         "Connecting", "Connected",    "JoiningZone",
                                    "JoiningArena", "Playing",    "Disconnected", "Downloading"};
    static_assert(sizeof(kStates) / sizeof(*kStates) == (size_t)ConnectState::Count);
    return kStates[(size_t)connect_state];
  }

  KeyState OnGetAsyncKeyState(int vKey) override { return {}; }

  bool OnPostUpdate(BOOL hasMsg, LPMSG lpMsg, HWND hWnd) override {
    if (!IsDownloadingMap()) return false;

    if (hasMsg && (lpMsg->message == WM_KEYDOWN || lpMsg->message == WM_KEYUP)) {
      if (lpMsg->wParam == VK_ESCAPE) {
        lpMsg->message = WM_NULL;
        return true;
      }
    }

    return false;
  }

  void OnMouseUp(const Vector2i& position, MouseButton button) override { painter.OnMouseUp(position, button); }
  void OnMouseMove(const Vector2i& position, MouseButtons buttons) override { painter.OnMouseMove(position, buttons); }

 private:
  inline bool IsDownloadingMap() {
    ConnectState connect_state = Fuse::Get().GetConnectState();

    return connect_state == ConnectState::Playing && !Fuse::Get().GetMap().IsLoaded();
  }

  Painter painter;
  float y = 288.0f;
};

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID reserved) {
  switch (dwReason) {
    case DLL_PROCESS_ATTACH: {
      Fuse::Get().RegisterHook(std::make_unique<HelloWorld>());
    } break;
  }
  return TRUE;
}
