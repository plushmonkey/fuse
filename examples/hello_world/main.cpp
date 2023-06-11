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
  void OnUpdate() override {
    std::string output = "Hello, world.";

    // Calculate a hue that cycles based on the current tick.
    float hue = ((GetTickCount() % 3000) / 3000.0f) * 360.0f;
    render::Color color = render::Color::FromHSV(hue, 0.8f, 0.6f);

    Player* player = Fuse::Get().GetPlayer();

    ConnectState connect_state = Fuse::Get().GetConnectState();

    if (connect_state == ConnectState::Playing && Fuse::Get().GetMap().IsLoaded()) {
      if (player && player->ship != 8) {
        output = std::format("Hello, {} ({}). You are at ({:.2f}, {:.2f}).", player->name, player->id,
                             player->position.x, player->position.y);

        // Check if the player is not in a safe.
        if (Fuse::Get().GetMap().GetTileId(player->position) != kSafeTileId) {
          // Render a line showing the direction the player is facing.
          Fuse::Get().GetRenderer().PushWorldLine(player->position, player->position + player->GetHeading() * 3.0f,
                                                  color);
        }
      }

      painter.Render(color);
    } else {
      const char* state_str = to_string(connect_state);

      output = std::format("ConnectState: {}", state_str);
      painter.Clear();
    }

    Fuse::Get().GetRenderer().PushText(output, Vector2f(300, 300), render::TextColor::Yellow);

    float line_length = (float)(output.size() * 8);
    Fuse::Get().GetRenderer().PushScreenLine(Vector2f(300, 312), Vector2f(300 + line_length, 312), color);
  }

  const char* to_string(ConnectState connect_state) {
    static const char* kStates[] = {"Menu",         "Connecting", "Connected",   "JoiningZone",
                                    "JoiningArena", "Playing",    "Disconnected"};
    return kStates[(size_t)connect_state];
  }

  KeyState OnGetAsyncKeyState(int vKey) override { return {}; }

  void OnMouseUp(const Vector2i& position, MouseButton button) override { painter.OnMouseUp(position, button); }
  void OnMouseMove(const Vector2i& position, MouseButtons buttons) override { painter.OnMouseMove(position, buttons); }

 private:
  Painter painter;
};

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID reserved) {
  switch (dwReason) {
    case DLL_PROCESS_ATTACH: {
      Fuse::Get().Inject();

      Fuse::Get().RegisterHook(std::make_unique<HelloWorld>());
    } break;
  }
  return TRUE;
}
