#include <fuse/Fuse.h>
#include <fuse/HookInjection.h>

using namespace fuse;

#include <fstream>

class HelloWorld final : public HookInjection {
 public:
  HelloWorld() { hello_log.open("hello.log", std::ios::out); }
  ~HelloWorld() { hello_log.close(); }

  void OnUpdate() override {
    Player* player = Fuse::Get().GetPlayer();

    if (!player) {
      hello_log << "Hello, world." << std::endl;
      return;
    }

    hello_log << "Hello, " << player->name << ". You are at (" << player->position.x << ", " << player->position.y
              << ")" << std::endl;
  }

  KeyState OnGetAsyncKeyState(int vKey) override { return {}; }

 private:
  std::ofstream hello_log;
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
