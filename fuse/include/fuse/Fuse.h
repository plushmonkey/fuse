#pragma once

#include <fuse/ClientSettings.h>
#include <fuse/ExeProcess.h>
#include <fuse/HookInjection.h>
#include <fuse/Map.h>
#include <fuse/Player.h>
#include <fuse/Weapon.h>
#include <fuse/render/Renderer.h>

#include <memory>
#include <string>
#include <vector>

namespace fuse {

enum class ConnectState : u32 {
  // This is the ConnectState when the client is on the menu and not attempting to join any zone.
  None = 0,
  // @4478D8
  Connecting,
  // @451962
  Connected,
  // @42284F
  // This is used for news and biller popups.
  JoiningZone,
  // @42E0C0
  // The client will transition to this while switching arenas.
  JoiningArena,
  // @436B36
  // This state also includes map downloading.
  Playing,
  // @451AFD
  // Normally this only occurs when the server sends the disconnect packet, but Fuse will set it
  // to this if no packet is received within 1500 ticks.
  // This is the same tick count that Continuum uses for the "No data" notification.
  Disconnected
};

struct ShipStatus {
  u32 recharge = 0;
  u32 thrust = 0;
  u32 speed = 0;
  u32 rotation = 0;
  u32 shrapnel = 0;
};

struct GameMemory {
  MemoryAddress module_base_continuum = 0;
  MemoryAddress module_base_menu = 0;
  MemoryAddress game_address = 0;
};

class Fuse {
 public:
  static Fuse& Get() {
    static Fuse instance;
    return instance;
  }

  void Inject();
  void Update();

  render::Renderer& GetRenderer() { return renderer; }
  ExeProcess& GetExeProcess() { return exe_process; }

  // This returns true if the client is on the menu and isn't attempting to connect to a zone.
  bool IsOnMenu() const;
  ConnectState GetConnectState() const;

  bool IsGameMenuOpen() const;
  void SetGameMenuOpen(bool open);

  Map& GetMap() { return map; }

  const ClientSettings& GetSettings() const;
  const ShipSettings& GetShipSettings() const;
  const ShipSettings& GetShipSettings(int ship) const;

  Player* GetPlayer() { return main_player; }
  const Player* GetPlayer() const { return main_player; }
  std::string GetName();

  void RegisterHook(std::unique_ptr<HookInjection> hook) { hooks.push_back(std::move(hook)); }

  const std::vector<std::unique_ptr<HookInjection>>& GetHooks() const { return hooks; }

  HWND GetGameWindowHandle();
  bool UpdateMemory();
  GameMemory& GetGameMemory() { return game_memory; }

 private:
  Fuse() {}

  void ReadPlayers();
  void ReadWeapons();

  std::vector<std::unique_ptr<HookInjection>> hooks;

  std::vector<Player> players;
  std::vector<Weapon> weapons;

  render::Renderer renderer;
  Map map;

  ShipStatus ship_status;
  Player* main_player = nullptr;
  u16 player_id = 0xFFFF;

  ExeProcess exe_process;
  GameMemory game_memory;

  bool initialized = false;
};

}  // namespace fuse
