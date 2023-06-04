#include <fuse/Fuse.h>
#include <fuse/HookInjection.h>

using namespace fuse;

#include <format>
#include <fstream>

class HelloWorld final : public HookInjection {
 public:
  void OnUpdate() override {
    std::string output = "Hello, world.";

    float hue = ((GetTickCount() % 3000) / 3000.0f) * 360.0f;
    render::Color color = render::Color::FromHSV(hue, 0.8f, 0.6f);

    Player* player = Fuse::Get().GetPlayer();

    if (player && player->ship != 8) {
      output = std::format("Hello, {} ({}). You are at ({:.2f}, {:.2f}).", player->name, player->id, player->position.x,
                           player->position.y);

      Fuse::Get().GetRenderer().PushWorldLine(player->position, player->position + player->GetHeading() * 3.0f, color);
    }

    Fuse::Get().GetRenderer().PushText(output, Vector2f(300, 300), render::TextColor::Yellow);

    float line_length = (float)(output.size() * 8);
    Fuse::Get().GetRenderer().PushScreenLine(Vector2f(300, 312), Vector2f(300 + line_length, 312), color);
  }

  KeyState OnGetAsyncKeyState(int vKey) override { return {}; }
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
