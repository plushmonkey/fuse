#include <fuse/Fuse.h>
#include <fuse/HookInjection.h>

using namespace fuse;

#include <format>
#include <fstream>

class HelloWorld final : public HookInjection {
 public:
  void OnUpdate() override {
    std::string output = "Hello, world.";

    Player* player = Fuse::Get().GetPlayer();

    if (player) {
      output = std::format("Hello, {}. You are at ({:.2f}, {:.2f}).", player->name, player->position.x, player->position.y);
    }

    Fuse::Get().GetRenderer().RenderText(output, Vector2f(300, 312), render::TextColor::Fuchsia);
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
