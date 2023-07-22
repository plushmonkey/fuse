#include <fuse/Args.h>
#include <fuse/Fuse.h>
#include <fuse/HookInjection.h>
//
#include <detours.h>

using namespace fuse;

constexpr u16 kInvalidIndex = 0xFFFF;

// Plugin for grabbing the profile index and zone index from the command line, and then automatically joining.
//
// Enable by launching like "Continuum.exe [profile_index] [zone_index]" such as "Continuum.exe 1 0"
//
// If a profile index and zone index are provided, then it will try to automatically join the zone.
class MenuController final : public HookInjection {
 public:
  const char* GetHookName() override { return "MenuController"; }

  MenuController(u16 profile_index, u16 zone_index) : profile_index(profile_index), zone_index(zone_index) {}

  bool OnMenuUpdate(BOOL hasMsg, LPMSG lpMsg, HWND hWnd) override {
    if (module_base_menu == 0) {
      module_base_menu = exe_process.GetModuleBase("menu040.dll");
      return false;
    }

    SetProfileIndex(profile_index);
    SetZoneIndex(zone_index);

    // If message is just a paint, then replace it with pressing enter button to join zone.
    if (ShouldJoin() && hasMsg && lpMsg->message == WM_PAINT) {
      lpMsg->message = WM_KEYDOWN;
      lpMsg->wParam = VK_RETURN;
      lpMsg->lParam = 0;

      // TODO: Use a timer to attempt to rejoin after a delay. This would make it auto-reconnect once leaving a zone.
      // Not currently implemented because I don't want to deal with accidental join spam and the other modal boxes that
      // can pop up on join.
      attempted_join = true;

      if (hasMsg) {
        ShowWindow(lpMsg->hwnd, SW_HIDE);
      }
      return true;
    }

    return false;
  }

 private:
  bool ShouldJoin() const { return !attempted_join && profile_index != kInvalidIndex && zone_index != kInvalidIndex; }

  void SetProfileIndex(u16 profile_index) {
    if (profile_index != kInvalidIndex) {
      MemoryAddress index_address = module_base_menu + 0x47FA0;

      *(u16*)index_address = profile_index;
    }
  }

  void SetZoneIndex(u16 zone_index) {
    if (zone_index != kInvalidIndex) {
      MemoryAddress index_address = module_base_menu + 0x47F9C;
      *(u16*)index_address = zone_index;
    }
  }

  ExeProcess exe_process;
  MemoryAddress module_base_menu = 0;

  u16 profile_index;
  u16 zone_index;
  bool attempted_join = false;
};

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID reserved) {
  switch (dwReason) {
    case DLL_PROCESS_ATTACH: {
      ArgList args = GetArguments();

      // TODO: Argument parser for setting options in any order
      if (args.size() > 1) {
        u16 profile_index = (u16)strtol(args[1].data(), nullptr, 10);
        u16 zone_index = kInvalidIndex;

        if (args.size() > 2) {
          zone_index = (u16)strtol(args[2].data(), nullptr, 10);
        }

        Fuse::Get().RegisterHook(std::make_unique<MenuController>(profile_index, zone_index));
      }
    } break;
  }

  return TRUE;
}
