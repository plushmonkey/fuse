#include <fuse/Platform.h>
//
#include <ddraw.h>
#include <detours.h>
#include <fuse/Fuse.h>

namespace fuse {

static SHORT(WINAPI* RealGetAsyncKeyState)(int vKey) = GetAsyncKeyState;
static BOOL(WINAPI* RealPeekMessageA)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax,
                                      UINT wRemoveMsg) = PeekMessageA;
static HRESULT(STDMETHODCALLTYPE* RealBlt)(LPDIRECTDRAWSURFACE, LPRECT, LPDIRECTDRAWSURFACE, LPRECT, DWORD, LPDDBLTFX);

SHORT WINAPI OverrideGetAsyncKeyState(int vKey) {
  KeyState final_state = {};

  for (auto& hook : Fuse::Get().GetHooks()) {
    KeyState state = hook->OnGetAsyncKeyState(vKey);

    if (state.forced) {
      final_state.forced = true;
      final_state.pressed = state.pressed;
    } else {
      final_state.pressed = final_state.pressed || state.pressed;
    }
  }

  if (final_state.forced && !final_state.pressed) {
    // If the result is forced and not pressed, then we want to return zero here to stop the actual keyboard detection
    // in the real function.
    return 0;
  } else if (final_state.pressed) {
    return (SHORT)0x8000;
  }

  return RealGetAsyncKeyState(vKey);
}

// This is used to hook into the main update loop in Continuum
BOOL WINAPI OverridePeekMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
  Fuse::Get().Update();

  for (auto& hook : Fuse::Get().GetHooks()) {
    hook->OnUpdate();
  }

  return RealPeekMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
}

HRESULT STDMETHODCALLTYPE OverrideBlt(LPDIRECTDRAWSURFACE surface, LPRECT dest_rect, LPDIRECTDRAWSURFACE next_surface,
                                      LPRECT src_rect, DWORD flags, LPDDBLTFX fx) {
  u32 graphics_addr = *(u32*)(0x4C1AFC) + 0x30;
  LPDIRECTDRAWSURFACE primary_surface = (LPDIRECTDRAWSURFACE) * (u32*)(graphics_addr + 0x40);
  LPDIRECTDRAWSURFACE back_surface = (LPDIRECTDRAWSURFACE) * (u32*)(graphics_addr + 0x44);

  // Check if flipping. I guess there's a full screen blit instead of flip when running without vsync?
  if (surface == primary_surface && next_surface == back_surface && fx == 0) {
    Fuse::Get().GetRenderer().Render();
  }

  return RealBlt(surface, dest_rect, next_surface, src_rect, flags, fx);
}

void Fuse::Inject() {
  if (!initialized) {
    module_base_continuum = exe_process.GetModuleBase("Continuum.exe");
    module_base_menu = exe_process.GetModuleBase("menu040.dll");

    game_address = exe_process.ReadU32(module_base_continuum + 0xC1AFC);

    DetourRestoreAfterWith();

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)RealGetAsyncKeyState, OverrideGetAsyncKeyState);
    DetourAttach(&(PVOID&)RealPeekMessageA, OverridePeekMessageA);
    DetourTransactionCommit();
    initialized = true;
  }
}

std::string Fuse::GetName() {
  const size_t ProfileStructSize = 2860;

  uint16_t profile_index = exe_process.ReadU32(module_base_menu + 0x47FA0) & 0xFFFF;
  MemoryAddress addr = exe_process.ReadU32(module_base_menu + 0x47A38) + 0x15;

  if (addr == 0) {
    return "";
  }

  addr += profile_index * ProfileStructSize;

  std::string name = exe_process.ReadString(addr, 23);

  name = name.substr(0, strlen(name.c_str()));

  return name;
}

bool Fuse::UpdateMemory() {
  if (!module_base_continuum) {
    module_base_continuum = exe_process.GetModuleBase("Continuum.exe");

    if (module_base_continuum == 0) return false;
  }

  if (!module_base_menu) {
    module_base_menu = exe_process.GetModuleBase("menu040.dll");

    if (module_base_menu == 0) return false;
  }

  game_address = exe_process.ReadU32(module_base_continuum + 0xC1AFC);

  if (game_address == 0) {
    main_player = nullptr;
    player_id = 0xFFFF;
    return false;
  }

  return true;
}

void Fuse::Update() {
  renderer.Reset();

  if (!UpdateMemory()) return;

  if (!renderer.injected) {
    u32 graphics_addr = *(u32*)(0x4C1AFC) + 0x30;
    if (graphics_addr) {
      LPDIRECTDRAWSURFACE surface = (LPDIRECTDRAWSURFACE) * (u32*)(graphics_addr + 0x44);

      if (surface) {
        void** vtable = (*(void***)surface);
        RealBlt = (HRESULT(STDMETHODCALLTYPE*)(LPDIRECTDRAWSURFACE surface, LPRECT, LPDIRECTDRAWSURFACE, LPRECT, DWORD,
                                               LPDDBLTFX))vtable[5];
        DetourRestoreAfterWith();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)RealBlt, OverrideBlt);
        DetourTransactionCommit();
        renderer.injected = true;
      }
    }
  }

  ReadPlayers();
  ReadWeapons();

  if (player_id == 0xFFFF) {
    for (auto& player : players) {
      if (player.name == GetName()) {
        player_id = player.id;
        main_player = &player;
        break;
      }
    }
  }
}

void Fuse::ReadPlayers() {
  const size_t kPosOffset = 0x04;
  const size_t kVelocityOffset = 0x10;
  const size_t kIdOffset = 0x18;
  const size_t kBountyOffset1 = 0x20;
  const size_t kBountyOffset2 = 0x24;
  const size_t kRotOffset = 0x3C;
  const size_t kShipOffset = 0x5C;
  const size_t kFreqOffset = 0x58;
  const size_t kStatusOffset = 0x60;
  const size_t kNameOffset = 0x6D;
  const size_t kEnergyOffset1 = 0x208;
  const size_t kEnergyOffset2 = 0x20C;

  players.clear();
  main_player = nullptr;
  player_id = 0xFFFF;

  if (game_address == 0) return;

  MemoryAddress base_addr = game_address + 0x127EC;
  MemoryAddress players_addr = base_addr + 0x884;
  MemoryAddress count_addr = base_addr + 0x1884;

  if (players_addr == 0 || count_addr == 0) return;

  size_t count = exe_process.ReadU32(count_addr) & 0xFFFF;

  std::string my_name = GetName();

  for (size_t i = 0; i < count; ++i) {
    MemoryAddress player_addr = exe_process.ReadU32(players_addr + (i * 4));

    if (!player_addr) continue;

    Player player;

    player.position.x = exe_process.ReadU32(player_addr + kPosOffset) / 1000.0f / 16.0f;
    player.position.y = exe_process.ReadU32(player_addr + kPosOffset + 4) / 1000.0f / 16.0f;

    player.velocity.x = exe_process.ReadI32(player_addr + kVelocityOffset) / 10.0f / 16.0f;
    player.velocity.y = exe_process.ReadI32(player_addr + kVelocityOffset + 4) / 10.0f / 16.0f;

    player.id = static_cast<uint16_t>(exe_process.ReadU32(player_addr + kIdOffset));
    player.discrete_rotation = static_cast<uint16_t>(exe_process.ReadU32(player_addr + kRotOffset) / 1000);

    player.ship = static_cast<uint8_t>(exe_process.ReadU32(player_addr + kShipOffset));
    player.frequency = static_cast<uint16_t>(exe_process.ReadU32(player_addr + kFreqOffset));

    player.status = static_cast<uint8_t>(exe_process.ReadU32(player_addr + kStatusOffset));

    player.name = exe_process.ReadString(player_addr + kNameOffset, 23);

    player.bounty = *(u32*)(player_addr + kBountyOffset1) + *(u32*)(player_addr + kBountyOffset2);

    if (player.name == my_name) {
      // Energy calculation @4485FA
      u32 energy1 = exe_process.ReadU32(player_addr + kEnergyOffset1);
      u32 energy2 = exe_process.ReadU32(player_addr + kEnergyOffset2);

      u32 combined = energy1 + energy2;
      u64 energy = ((combined * (u64)0x10624DD3) >> 32) >> 6;

      player.energy = static_cast<uint16_t>(energy);

      // @448D37
      ship_status.rotation = *(u32*)(player_addr + 0x278) + *(u32*)(player_addr + 0x274);
      ship_status.recharge = *(u32*)(player_addr + 0x1E8) + *(u32*)(player_addr + 0x1EC);
      ship_status.shrapnel = *(u32*)(player_addr + 0x2A8) + *(u32*)(player_addr + 0x2AC);
      ship_status.thrust = *(u32*)(player_addr + 0x244) + *(u32*)(player_addr + 0x248);
      ship_status.speed = *(u32*)(player_addr + 0x350) + *(u32*)(player_addr + 0x354);

      player_id = player.id;
    } else {
      u32 first = *(u32*)(player_addr + 0x150);
      u32 second = *(u32*)(player_addr + 0x154);

      player.energy = static_cast<uint16_t>(first + second);
    }

    players.emplace_back(player);
  }

  // Get pointer to the main player after updating the player list because std::vector does not preserve address
  for (Player& player : players) {
    if (player.id == player_id) {
      main_player = &player;
      break;
    }
  }
}

void Fuse::ReadWeapons() {
  weapons.clear();

  if (game_address == 0) return;

  // Grab the address to the main player structure
  MemoryAddress player_addr = *(MemoryAddress*)(game_address + 0x13070);

  if (player_addr == 0) return;

  // Follow a pointer that leads to weapon data
  MemoryAddress ptr = *(MemoryAddress*)(player_addr + 0x0C);

  if (ptr == 0) return;

  u32 weapon_count = *(u32*)(ptr + 0x1DD0) + *(u32*)(ptr + 0x1DD4);
  MemoryAddress weapon_ptrs = (ptr + 0x21F4);

  for (size_t i = 0; i < weapon_count; ++i) {
    MemoryAddress weapon_address = *(MemoryAddress*)(weapon_ptrs + i * 4);

    Weapon weapon = *(Weapon*)(weapon_address);

    weapons.emplace_back(weapon);
  }
}

}  // namespace fuse
