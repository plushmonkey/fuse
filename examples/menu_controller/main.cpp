#include <fuse/Args.h>
#include <fuse/Fuse.h>
#include <fuse/HookInjection.h>

using namespace fuse;

// Currently controls the selected profile.
// TODO: Control selected zone and auto-connect.
class MenuController final : public HookInjection {
 public:
  MenuController(u16 index) : index(index) {}

  void OnMenuUpdate() override {
    if (module_base_menu == 0) {
      module_base_menu = exe_process.GetModuleBase("menu040.dll");
      return;
    }

    SetProfileIndex(index);
  }

 private:
  void SetProfileIndex(u16 index) {
    MemoryAddress index_address = module_base_menu + 0x47FA0;

    *(u16*)index_address = index;
  }

  ExeProcess exe_process;
  MemoryAddress module_base_menu = 0;

  u16 index;
};

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID reserved) {
  switch (dwReason) {
    case DLL_PROCESS_ATTACH: {
      ArgList args = GetArguments();

      // TODO: Argument parser for setting options in any order
      if (args.size() > 1) {
        Fuse::Get().Inject();

        u16 index = (u16)strtol(args[1].data(), nullptr, 10);

        Fuse::Get().RegisterHook(std::make_unique<MenuController>(index));
      }
    } break;
  }

  return TRUE;
}
