#pragma once

#include <fuse/ExeProcess.h>
#include <fuse/HookInjection.h>
#include <fuse/Player.h>
#include <fuse/Weapon.h>

#include <memory>
#include <string>
#include <vector>

namespace fuse {

struct ShipStatus {
  u32 recharge = 0;
  u32 thrust = 0;
  u32 speed = 0;
  u32 rotation = 0;
  u32 shrapnel = 0;
};

class Fuse {
 public:
  static Fuse& Get() {
    static Fuse instance;
    return instance;
  }

  void Inject();
  void Update();

  Player* GetPlayer() { return main_player; }
  std::string GetName();

  void RegisterHook(std::unique_ptr<HookInjection> hook) { hooks.push_back(std::move(hook)); }

  const std::vector<std::unique_ptr<HookInjection>>& GetHooks() const { return hooks; }

 private:
  Fuse() {}

  void ReadPlayers();
  void ReadWeapons();

  std::vector<std::unique_ptr<HookInjection>> hooks;

  std::vector<Player> players;
  std::vector<Weapon> weapons;

  ShipStatus ship_status;
  Player* main_player = nullptr;
  u16 player_id = 0xFFFF;

  ExeProcess exe_process;
  MemoryAddress module_base_continuum = 0;
  MemoryAddress module_base_menu = 0;
  MemoryAddress game_address = 0;

  bool initialized = false;
};

}  // namespace fuse
