#include <fuse/Platform.h>
//
#include <detours.h>
#include <fuse/Fuse.h>
#include <fuse/render/GDIRenderer.h>

namespace fuse {

Fuse::Fuse() {
  renderer = std::make_unique<render::GDIRenderer>();
}

Fuse& Fuse::Get() {
  static Fuse instance;
  return instance;
}

static SHORT(WINAPI* RealGetAsyncKeyState)(int vKey) = GetAsyncKeyState;
static BOOL(WINAPI* RealPeekMessageA)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax,
                                      UINT wRemoveMsg) = PeekMessageA;
static BOOL(WINAPI* RealGetMessageA)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax) = GetMessageA;

static int(WINAPI* RealMessageBoxA)(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) = MessageBoxA;

int WINAPI OverrideMessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
  bool suppress = false;

  for (auto& hook : Fuse::Get().GetHooks()) {
    if (hook->OnMessageBox(lpText, lpCaption, uType)) {
      suppress = true;
    }
  }

  if (suppress) {
    return 0;
  }

  return RealMessageBoxA(hWnd, lpText, lpCaption, uType);
}

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

  bool bypass = false;

  if (lpMsg->message == WM_QUIT) {
    for (auto& hook : Fuse::Get().GetHooks()) {
      hook->OnQuit();
    }

    Fuse::Get().ClearHooks();
  }

  // Run OnPeekMessage for each hook and stop if any try to bypass it.
  // If they want to bypass it then they are trying to return their own MSG.
  for (auto& hook : Fuse::Get().GetHooks()) {
    if (hook->OnPeekMessage(lpMsg, hWnd)) {
      bypass = true;
      break;
    }
  }

  for (auto& hook : Fuse::Get().GetHooks()) {
    hook->OnUpdate();
  }

  BOOL result = bypass;

  // Only run the real PeekMessage if a hook didn't try to inject their own message.
  if (!result) {
    // Continuum seems to have an odd update loop. Usual game loops just use PeekMessage with remove set, but Continuum
    // runs it with false. When it finds a message in the queue, it runs GetMessage to ensure it doesn't block. This
    // means we need to handle things in a non-standard way by filtering out the messages we don't want to process by
    // calling it again with wRemoveMsg set to true.
    result = RealPeekMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
  }

  if (result) {
    Fuse::Get().HandleWindowsEvent(lpMsg, hWnd);

    if (lpMsg->message == WM_NULL) {
      // Filter out this message since we don't actually want Continuum to process it.
      RealPeekMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, TRUE);
      // Tell Continuum we didn't have a message.
      return false;
    }
  }

  for (auto& hook : Fuse::Get().GetHooks()) {
    if (hook->OnPostUpdate(result, lpMsg, hWnd)) {
      result = TRUE;
    }
  }

  return result;
}

BOOL WINAPI OverrideGetMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax) {
  Fuse::Get().UpdateMemory();

  bool bypass = false;

  if (lpMsg->message == WM_QUIT) {
    for (auto& hook : Fuse::Get().GetHooks()) {
      hook->OnQuit();
    }

    Fuse::Get().ClearHooks();
  }

  // Run OnGetMessage for each hook and stop if any try to bypass it.
  // If they want to bypass it then they are trying to return their own MSG.
  for (auto& hook : Fuse::Get().GetHooks()) {
    if (hook->OnGetMessage(lpMsg, hWnd)) {
      bypass = true;
      break;
    }
  }

  BOOL result = bypass;

  // Only run the real GetMessage if a hook didn't try to inject their own message.
  if (!result) {
    result = RealGetMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
  }

  for (auto& hook : Fuse::Get().GetHooks()) {
    if (Fuse::Get().IsOnMenu()) {
      if (hook->OnMenuUpdate(result, lpMsg, hWnd)) {
        result = TRUE;
      }
    } else {
      if (hook->OnPostUpdate(result, lpMsg, hWnd)) {
        result = TRUE;
      }
    }
  }

  return result;
}

static inline Vector2i GetMousePosition(LPARAM lParam) {
  return Vector2i((s32)LOWORD(lParam), (s32)HIWORD(lParam));
}

void Fuse::HandleWindowsEvent(LPMSG msg, HWND hWnd) {
  WPARAM wParam = msg->wParam;
  LPARAM lParam = msg->lParam;

  for (auto& hook : Fuse::Get().GetHooks()) {
    if (hook->OnWindowsEvent(*msg, wParam, lParam)) {
      // Swallow the message so Continuum doesn't handle it.
      // This is used for filtering out keypresses that hooks might be handling.
      msg->message = WM_NULL;
    }
  }

  switch (msg->message) {
    case WM_MOUSEMOVE: {
      Vector2i position = GetMousePosition(lParam);

      for (auto& hook : Fuse::Get().GetHooks()) {
        hook->OnMouseMove(position, MouseButtons(wParam));
      }
    } break;
    case WM_LBUTTONDOWN: {
      for (auto& hook : Fuse::Get().GetHooks()) {
        Vector2i position = GetMousePosition(lParam);

        hook->OnMouseDown(position, MouseButton::Left);
      }
    } break;
    case WM_RBUTTONDOWN: {
      for (auto& hook : Fuse::Get().GetHooks()) {
        Vector2i position = GetMousePosition(lParam);

        hook->OnMouseDown(position, MouseButton::Right);
      }
    } break;
    case WM_MBUTTONDOWN: {
      for (auto& hook : Fuse::Get().GetHooks()) {
        Vector2i position = GetMousePosition(lParam);

        hook->OnMouseDown(position, MouseButton::Middle);
      }
    } break;
    case WM_LBUTTONUP: {
      for (auto& hook : Fuse::Get().GetHooks()) {
        Vector2i position = GetMousePosition(lParam);

        hook->OnMouseUp(position, MouseButton::Left);
      }
    } break;
    case WM_RBUTTONUP: {
      for (auto& hook : Fuse::Get().GetHooks()) {
        Vector2i position = GetMousePosition(lParam);

        hook->OnMouseUp(position, MouseButton::Right);
      }
    } break;
    case WM_MBUTTONUP: {
      for (auto& hook : Fuse::Get().GetHooks()) {
        Vector2i position = GetMousePosition(lParam);

        hook->OnMouseUp(position, MouseButton::Middle);
      }
    } break;
    default: {
    } break;
  }
}

void Fuse::Inject() {
  if (!initialized) {
    game_memory.module_base_continuum = exe_process.GetModuleBase("Continuum.exe");
    game_memory.module_size_continuum = exe_process.GetModuleSize("Continuum.exe");
    game_memory.module_base_menu = exe_process.GetModuleBase("menu040.dll");

    game_memory.game_address = exe_process.ReadU32(game_memory.module_base_continuum + 0xC1AFC);

    DetourRestoreAfterWith();

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)RealGetAsyncKeyState, OverrideGetAsyncKeyState);
    DetourAttach(&(PVOID&)RealPeekMessageA, OverridePeekMessageA);
    DetourAttach(&(PVOID&)RealGetMessageA, OverrideGetMessageA);
    DetourAttach(&(PVOID&)RealMessageBoxA, OverrideMessageBoxA);
    DetourTransactionCommit();
    initialized = true;
  }
}

std::string Fuse::GetZoneName() const {
  constexpr size_t kZoneStructSize = 0xD8;
  constexpr size_t kZoneNameOffset = 0x92;

  if (!game_memory.game_address) return "";

  u32 zone_base = *(u32*)(game_memory.module_base_menu + 0x46C58);
  u32 zone_index = *(u32*)(game_memory.module_base_menu + 0x47F9C);

  char* zone_name = (char*)(zone_base + (kZoneStructSize * zone_index) + kZoneNameOffset);

  return std::string(zone_name);
}

std::string Fuse::GetArenaName() const {
  if (!game_memory.game_address) return "";

  char* map_name = (char*)(game_memory.game_address + 0x127ec + 0x1BBA4);

  if (*map_name == 0) {
    return std::string("(public)");
  }

  return std::string(map_name);
}

std::string Fuse::GetMapName() const {
  if (!game_memory.game_address) return "";

  char* map_name = (char*)((*(u32*)(game_memory.game_address + 0x127ec + 0x6C4)) + 0x01);

  return std::string(map_name);
}

const ClientSettings& Fuse::GetSettings() const {
  static ClientSettings empty_settings = {};

  if (!game_memory.game_address) {
    return empty_settings;
  }

  MemoryAddress addr = game_memory.game_address + 0x127EC + 0x1AE70;

  return *reinterpret_cast<ClientSettings*>(addr);
}

const ShipSettings& Fuse::GetShipSettings() const {
  auto player = GetPlayer();

  if (player == nullptr) {
    return GetShipSettings(8);
  }

  return GetShipSettings(player->ship);
}

const ShipSettings& Fuse::GetShipSettings(int ship) const {
  static ShipSettings empty_settings = {};

  if (ship < 0 || ship >= 8) {
    return empty_settings;
  }

  return GetSettings().ShipSettings[ship];
}

std::string Fuse::GetName() {
  const size_t ProfileStructSize = 2860;

  uint16_t profile_index = exe_process.ReadU32(game_memory.module_base_menu + 0x47FA0) & 0xFFFF;
  MemoryAddress addr = exe_process.ReadU32(game_memory.module_base_menu + 0x47A38) + 0x15;

  if (addr == 0) {
    return "";
  }

  addr += profile_index * ProfileStructSize;

  std::string name = exe_process.ReadString(addr, 23);

  name = name.substr(0, strlen(name.c_str()));

  return name;
}

HWND Fuse::GetGameWindowHandle() {
  HWND handle = 0;

  if (game_memory.game_address) {
    handle = *(HWND*)(game_memory.game_address + 0x8C);
  }

  return handle;
}

bool Fuse::IsOnMenu() const {
  if (game_memory.module_base_menu == 0) return true;

  return *(u8*)(game_memory.module_base_menu + 0x47a84) == 0;
}

ConnectState Fuse::GetConnectState() const {
  if (IsOnMenu()) return ConnectState::None;

  if (game_memory.game_address == 0) return ConnectState::None;

  ConnectState state = *(ConnectState*)(game_memory.game_address + 0x127EC + 0x588);

  if (state == ConnectState::Playing) {
    // Check if the client has timed out.
    u32 ticks_since_recv = *(u32*)(game_memory.game_address + 0x127EC + 0x590);

    // @453420
    if (ticks_since_recv > 1500) {
      return ConnectState::Disconnected;
    }

    if (!GetMap().IsLoaded()) {
      return ConnectState::Downloading;
    }
  }

  return state;
}

bool Fuse::IsGameMenuOpen() const {
  if (game_memory.game_address == 0) return false;

  ConnectState connect_state = GetConnectState();

  if (connect_state == ConnectState::Playing || connect_state == ConnectState::Disconnected) {
    return *(u8*)(game_memory.game_address + 0x127EC + 0x74D);
  }

  return false;
}

void Fuse::SetGameMenuOpen(bool open) {
  if (game_memory.game_address == 0) return;

  ConnectState connect_state = GetConnectState();

  if (connect_state == ConnectState::Playing || connect_state == ConnectState::Disconnected) {
    *(u8*)(game_memory.game_address + 0x127EC + 0x74D) = (u8)open;
  }
}

bool Fuse::UpdateMemory() {
  if (!game_memory.module_base_continuum) {
    game_memory.module_base_continuum = exe_process.GetModuleBase("Continuum.exe");

    if (game_memory.module_base_continuum == 0) return false;

    game_memory.module_size_continuum = exe_process.GetModuleSize("Continuum.exe");
  }

  if (!game_memory.module_base_menu) {
    game_memory.module_base_menu = exe_process.GetModuleBase("menu040.dll");

    if (game_memory.module_base_menu == 0) return false;
  }

  game_memory.game_address = exe_process.ReadU32(game_memory.module_base_continuum + 0xC1AFC);

  if (game_memory.game_address == 0) {
    main_player = nullptr;
    player_id = 0xFFFF;
    return false;
  }

  return true;
}

void Fuse::Update() {
  map = Map();

  if (!UpdateMemory()) return;

  u32* map_addr = (u32*)(*(u32*)(0x4C1AFC) + 0x127ec + 0x1d6d0);

  if (map_addr) {
    map = Map((u8*)*map_addr);
  }

  renderer->OnNewFrame();

  ReadPlayers();
  ReadWeapons();

  if (player_id == 0xFFFF) {
    for (auto& player : players) {
      if (player.GetName() == GetName()) {
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

  if (game_memory.game_address == 0) return;

  MemoryAddress base_addr = game_memory.game_address + 0x127EC;
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

    player.SetName(exe_process.ReadString(player_addr + kNameOffset, 23));

    player.bounty = *(u32*)(player_addr + kBountyOffset1) + *(u32*)(player_addr + kBountyOffset2);

    u32 respawn_time = *(u32*)(player_addr + 0x40);  // This is the explode timer.

    if (respawn_time > 0) {
      respawn_time += GetSettings().EnterDelay;  // Add enter delay to explode timer to get full respawn timer.
    } else {
      respawn_time = *(u32*)(player_addr + 0x144);  // Explode is already finished so just use the actual respawn timer.
    }

    player.respawn_time = respawn_time;

    if (player.GetName() == my_name) {
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

      ship_status.guns = *(u32*)(player_addr + 0x298) + *(u32*)(player_addr + 0x29C);
      ship_status.bombs = *(u32*)(player_addr + 0x2A0) + *(u32*)(player_addr + 0x2A4);

      ship_status.repels = *(u32*)(player_addr + 0x2B0) + *(u32*)(player_addr + 0x2B4);
      ship_status.bursts = *(u32*)(player_addr + 0x2B8) + *(u32*)(player_addr + 0x2BC);
      ship_status.bricks = *(u32*)(player_addr + 0x2C0) + *(u32*)(player_addr + 0x2C4);
      ship_status.rockets = *(u32*)(player_addr + 0x2C8) + *(u32*)(player_addr + 0x2CC);
      ship_status.thors = *(u32*)(player_addr + 0x2D0) + *(u32*)(player_addr + 0x2D4);
      ship_status.decoys = *(u32*)(player_addr + 0x2D8) + *(u32*)(player_addr + 0x2DC);
      ship_status.portals = *(u32*)(player_addr + 0x2E0) + *(u32*)(player_addr + 0x2E4);

      u32 capabilities = *(u32*)(player_addr + 0x2EC);
      u32 cloak_stealth = *(u32*)(player_addr + 0x2E8);

      ship_status.capability.stealth = (cloak_stealth & (1 << 17)) != 0;
      ship_status.capability.cloak = (cloak_stealth & (1 << 30)) != 0;
      ship_status.capability.antiwarp = (capabilities & (1 << 7)) != 0;
      ship_status.capability.xradar = (capabilities & (1 << 9)) != 0;
      ship_status.capability.multifire = (capabilities & (1 << 15)) != 0;
      ship_status.capability.bouncing_bullets = (capabilities & (1 << 19)) != 0;
      ship_status.capability.proximity = (capabilities & (1 << 26)) != 0;

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

  if (game_memory.game_address == 0) return;

  // Grab the address to the main player structure
  MemoryAddress player_addr = *(MemoryAddress*)(game_memory.game_address + 0x13070);

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

CallAddress Fuse::GetCallAddress(u32 skip) {
  if (!game_memory.module_base_continuum) return {};

  u32 module_end = game_memory.module_base_continuum + game_memory.module_size_continuum;

  void* frames[32];
  WORD frame_count = RtlCaptureStackBackTrace(skip, FUSE_ARRAY_SIZE(frames), frames, 0);

  for (u32 i = 0; i < frame_count; ++i) {
    MemoryAddress current = (MemoryAddress)frames[i];
    if (current >= game_memory.module_base_continuum && current < module_end) {
      CallAddress result = {};

      result.address = current;
      result.frame_index = i;

      return result;
    }
  }

  return {};
}

}  // namespace fuse
