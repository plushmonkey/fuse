#include <fuse/Fuse.h>
#include <fuse/HookInjection.h>

#include <algorithm>
#include <optional>
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

struct Notification {
  std::string message;
  render::TextColor color;
  Tick tick_end = 0;

  Notification(std::string_view msg, render::TextColor color, u32 duration)
      : message(msg), color(color), tick_end(duration + GetCurrentTick()) {}

  inline bool IsActive() const { return tick_end >= GetCurrentTick(); }
};

struct FilterListener {
  virtual void OnFilterChange() = 0;
};

struct FilterSelector {
  enum class State { Disabled = 0, Insert, Locked };

  void Render() {
    if (state == State::Disabled) return;

    render::Color fill_color = render::Color::FromRGB(33, 33, 33);
    render::Color border_color = render::Color::FromRGB(127, 127, 127);

    constexpr float kSettingsWindowWidth = 500;

    std::string filter_text = "Filter: " + std::string(filter);
    float extent_width = filter_text.size() * 8.0f + 3.0f;

    Vector2f surface_size = Fuse::Get().GetRenderer().GetSurfaceSize();
    Vector2f extent = Vector2f(extent_width, 9);

    Vector2f position((surface_size.x / 2) - (kSettingsWindowWidth / 2), 322);

    Fuse::Get().GetRenderer().PushScreenQuad(position, extent, fill_color);
    Fuse::Get().GetRenderer().PushScreenBorder(position, extent, border_color, 3.0f);
    Fuse::Get().GetRenderer().PushScreenQuad(position - Vector2f(0, 3), extent - Vector2f(0, 3), fill_color);

    render::TextColor color = state == State::Locked ? render::TextColor::Green : render::TextColor::Blue;
    Fuse::Get().GetRenderer().PushText(filter_text, Vector2f(position.x + 2, position.y - 3), color);
  }

  bool OnWindowsEvent(MSG msg, WPARAM wParam, LPARAM lParam) {
    if (state != State::Insert) return false;

    if (msg.message == WM_KEYDOWN && wParam == VK_RETURN) {
      state = State::Locked;
      return true;
    }

    if (msg.message == WM_KEYDOWN && wParam == VK_BACK && (GetAsyncKeyState(VK_CONTROL) & 0x8000)) {
      insert_index = 0;
      filter[insert_index] = 0;
      if (listener) listener->OnFilterChange();
      return true;
    }

    if (msg.message == WM_CHAR) {
      if (insert_index >= FUSE_ARRAY_SIZE(filter) - 1) return false;

      u8 codepoint = (u8)wParam;

      if ((codepoint >= 'A' && codepoint <= 'Z') || (codepoint >= 'a' && codepoint <= 'z')) {
        if (codepoint <= 'Z') codepoint += 0x20;  // Always use lowercase in filter.

        filter[insert_index++] = codepoint;
        filter[insert_index] = 0;

        if (listener) listener->OnFilterChange();
      }

      if (codepoint == VK_BACK && insert_index > 0) {
        --insert_index;
        filter[insert_index] = 0;

        if (listener) listener->OnFilterChange();
      }

      return true;
    }

    return false;
  }

  void Toggle() {
    if (state == State::Disabled) {
      state = State::Insert;
    } else {
      state = State::Disabled;
    }

    bool changed = insert_index != 0;

    insert_index = 0;
    filter[0] = 0;

    if (changed && listener) listener->OnFilterChange();
  }

  void Disable() {
    if (state != State::Disabled) {
      Toggle();
    }
  }

  inline bool IsEnabled() const { return state != State::Disabled; }

  size_t insert_index = 0;
  char filter[32] = {};
  State state = State::Disabled;
  FilterListener* listener = nullptr;
};

struct SettingsWindow : public FilterListener {
  void Toggle() {
    open = !open;

    if (!open) return;

    selected_index = 0;
    view_index = 0;
    notification = {};

    PopulateSettings();

    filter.listener = this;
    filter.Disable();

    arena_name = Fuse::Get().GetArenaName();
  }

  bool OnWindowsEvent(MSG msg, WPARAM wParam, LPARAM lParam) {
    if (filter.OnWindowsEvent(msg, wParam, lParam)) {
      return true;
    }

    if (msg.message != WM_KEYDOWN) return false;

    switch (wParam) {
      case VK_OEM_5: {
        filter.Toggle();
        return true;
      } break;
      case VK_ESCAPE: {
        Toggle();
        return true;
      } break;
      case VK_NEXT: {
        if (GetAsyncKeyState(VK_SHIFT)) {
          selected_index += item_view_count;
        } else {
          ++selected_index;
        }

        if (selected_index >= settings.size()) {
          selected_index = settings.size() - 1;
        }

        return true;
      } break;
      case VK_PRIOR: {
        if (GetAsyncKeyState(VK_SHIFT)) {
          selected_index -= item_view_count;
        } else {
          --selected_index;
        }

        if (selected_index >= settings.size()) {
          selected_index = 0;
        }

        return true;
      } break;
      case VK_END: {
        Save();
        return true;
      } break;
      default: {
      } break;
    }

    return false;
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

    if (notification && notification->IsActive()) {
      Fuse::Get().GetRenderer().PushText(notification->message, Vector2f(text_x, text_y), notification->color);
    } else {
      notification = {};

      std::string filename = Fuse::Get().GetArenaName() + ".conf";
      std::string save_message = "Save " + filename + ": End";

      Fuse::Get().GetRenderer().PushText("Move: PgUp/PgDn Filter: \\", Vector2f(text_x, text_y),
                                         render::TextColor::Green);
      Fuse::Get().GetRenderer().PushText(save_message, Vector2f(text_x + extent.x - 1, text_y),
                                         render::TextColor::Green, render::RenderText_AlignRight);
    }

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

    filter.Render();
  }

  void PopulateSettings() {
    settings.clear();

    MemorySettingsWriter writer(settings);

    writer.Write(Fuse::Get().GetSettings());
  }

  void OnFilterChange() override {
    selected_index = 0;
    view_index = 0;

    PopulateSettings();

    if (filter.insert_index == 0) return;

    settings.erase(std::remove_if(settings.begin(), settings.end(),
                                  [&](const Setting& setting) {
                                    std::string lower_category = setting.category;
                                    std::string lower_name = setting.name;

                                    std::transform(lower_category.begin(), lower_category.end(), lower_category.begin(),
                                                   ::tolower);
                                    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);

                                    return lower_category.find(filter.filter) == std::string::npos &&
                                           lower_name.find(filter.filter) == std::string::npos;
                                  }),
                   settings.end());
  }

  void Save() {
    std::string arena_name = Fuse::Get().GetArenaName();

    FileSettingsWriter writer(arena_name + ".conf");

    if (writer.Write(Fuse::Get().GetSettings())) {
      notification =
          Notification("Successfully exported config to " + arena_name + ".conf", render::TextColor::Green, 500);
    } else {
      notification = Notification("Failed to export config to " + arena_name + ".conf", render::TextColor::Red, 500);
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

  FilterSelector filter;

  std::optional<Notification> notification;
  std::vector<Setting> settings;

  std::string arena_name;
};

class SettingsHook final : public HookInjection {
 public:
  const char* GetHookName() override { return "Settings"; }

  inline bool ShouldHide() const {
    if (!window.open) return false;

    if (Fuse::Get().GetConnectState() != ConnectState::Playing) return true;
    if (Fuse::Get().GetArenaName() != window.arena_name) return true;

    return false;
  }

  void OnUpdate() override {
    if (ShouldHide()) {
      window.open = false;
      return;
    }

    window.Render();
  }

  KeyState OnGetAsyncKeyState(int vKey) override { return {}; }

  bool OnWindowsEvent(MSG msg, WPARAM wParam, LPARAM lParam) override {
    switch (msg.message) {
      case WM_KEYDOWN: {
        if (wParam == VK_F12 && Fuse::Get().IsGameMenuOpen()) {
          window.Toggle();
          Fuse::Get().SetGameMenuOpen(false);
          return true;
        }
      } break;
      default: {
      } break;
    }

    if (window.open) {
      if (window.OnWindowsEvent(msg, wParam, lParam)) {
        return true;
      }
    }

    return false;
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
