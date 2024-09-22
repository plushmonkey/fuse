#include <fuse/Fuse.h>
#include <fuse/HookInjection.h>

#include <string>
#include <vector>

#include "SettingsWriter.h"

using namespace fuse;

struct Setting {
  std::string category;
  std::string name;
  std::string value;

  Setting(std::string_view category, std::string_view name, std::string_view value)
      : category(category), name(name), value(value) {}
};

struct MemorySettingsWriter : public SettingsWriter {
  MemorySettingsWriter(std::vector<Setting>& target) : output(target) {}

  void WriteHeader(std::string_view category) override { this->category = category; }

  void WriteSetting(std::string_view name, std::string_view value) override {
    output.emplace_back(category, name, value);
  }

  std::string category;
  std::vector<Setting>& output;
};

struct SettingsWindow {
  void Toggle() {
    open = !open;

    if (!open) return;

    selected_index = 0;
    view_index = 0;

    PopulateSettings();
  }

  void Render() {
    if (!open) return;

    render::Color fill_color = render::Color::FromRGB(33, 33, 33);
    render::Color border_color = render::Color::FromRGB(127, 127, 127);

    surface_size = Fuse::Get().GetRenderer().GetSurfaceSize();
    extent = Vector2f(500, 314);

    Vector2f position((surface_size.x / 2) - (extent.x / 2), 5);

    Fuse::Get().GetRenderer().PushScreenQuad(position, extent, fill_color);
    Fuse::Get().GetRenderer().PushScreenBorder(position, extent, border_color, 3.0f);

    text_x = position.x + 1;
    text_y = position.y + 2;

    std::string filename = Fuse::Get().GetArenaName() + ".conf";
    std::string save_message = "Save to " + filename + ": End";

    Fuse::Get().GetRenderer().PushText("Move: PgUp/PgDown", Vector2f(text_x, text_y), render::TextColor::Green);
    Fuse::Get().GetRenderer().PushText(save_message, Vector2f(text_x + extent.x, text_y), render::TextColor::Green,
                                       render::RenderText_AlignRight);
    text_y += 12;

    item_view_count = (size_t)((extent.y - 12) / 12);

    if (selected_index >= view_index + item_view_count) {
      view_index = selected_index - item_view_count + 1;
    } else if (selected_index < view_index) {
      view_index = selected_index;
    }

    for (size_t i = view_index; i < settings.size() && i < view_index + item_view_count; ++i) {
      const auto& setting = settings[i];
      render::TextColor text_color = render::TextColor::White;

      if (i == selected_index) {
        text_color = render::TextColor::Red;
      }

      Fuse::Get().GetRenderer().PushText(setting.category, Vector2f(text_x, text_y), text_color);

      float name_x = text_x + extent.x / 4;

      Fuse::Get().GetRenderer().PushText(setting.name, Vector2f(name_x, text_y), text_color);
      Fuse::Get().GetRenderer().PushText(setting.value, Vector2f(text_x + extent.x - 1, text_y), text_color,
                                         render::RenderText_AlignRight);

      text_y += 12.0f;
    }
  }

  void PopulateSettings() {
    settings.clear();

    MemorySettingsWriter writer(settings);

    writer.Write(Fuse::Get().GetSettings());
  }

  void Save() {
    std::string arena_name = Fuse::Get().GetArenaName();

    FileSettingsWriter writer(arena_name + ".conf");

    if (writer.Write(Fuse::Get().GetSettings())) {
      Toggle();
    }
  }

  Vector2f surface_size;
  Vector2f extent;

  float text_x;
  float text_y;

  bool open = false;
  size_t selected_index = 0;
  size_t view_index = 0;
  size_t item_view_count = 25;

  std::vector<Setting> settings;
};

class SettingsHook final : public HookInjection {
 public:
  const char* GetHookName() override { return "Settings"; }

  void OnUpdate() override {
    auto self = Fuse::Get().GetPlayer();
    if (!self) return;

    window.Render();
  }

  KeyState OnGetAsyncKeyState(int vKey) override { return {}; }

  void OnWindowsEvent(MSG msg, WPARAM wParam, LPARAM lParam) override {
    switch (msg.message) {
      case WM_KEYDOWN: {
        if (wParam == VK_F12 && Fuse::Get().IsGameMenuOpen()) {
          window.Toggle();
        }

        if (window.open) {
          switch (wParam) {
            case VK_ESCAPE: {
              window.Toggle();
              // Set game menu open so it closes when Continuum handles the escape key.
              Fuse::Get().SetGameMenuOpen(true);
            } break;
            case VK_NEXT: {
              if (GetAsyncKeyState(VK_SHIFT)) {
                window.selected_index += window.item_view_count;
              } else {
                ++window.selected_index;
              }

              if (window.selected_index >= window.settings.size()) {
                window.selected_index = window.settings.size() - 1;
              }
            } break;
            case VK_PRIOR: {
              if (GetAsyncKeyState(VK_SHIFT)) {
                window.selected_index -= window.item_view_count;
              } else {
                --window.selected_index;
              }

              if (window.selected_index >= window.settings.size()) {
                window.selected_index = 0;
              }
            } break;
            case VK_END: {
              window.Save();
            } break;
            default: {
            } break;
          }
        }

      } break;
    }
  }

  SettingsWindow window;
};

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID reserved) {
  switch (dwReason) {
    case DLL_PROCESS_ATTACH: {
      Fuse::Get().RegisterHook(std::make_unique<SettingsHook>());
    } break;
  }
  return TRUE;
}
