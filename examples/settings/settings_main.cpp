#include <fuse/Fuse.h>
#include <fuse/HookInjection.h>

#include <string>
#include <vector>

using namespace fuse;

struct Setting {
  std::string_view category;
  std::string_view name;
  std::string value;

  Setting(std::string_view category, std::string_view name, std::string_view value)
      : category(category), name(name), value(value) {}
};

struct SettingsWindow {
  void Toggle() {
    open = !open;

    if (!open) return;

    selected_index = 0;
    view_index = 0;

    PopulateSettings();
  }

  void Render() {
    if (!open) return;

    render::Color fill_color = render::Color::FromRGB(33, 33, 33);
    render::Color border_color = render::Color::FromRGB(127, 127, 127);

    surface_size = Fuse::Get().GetRenderer().GetSurfaceSize();
    extent = Vector2f(500, 300);

    Vector2f position((surface_size.x / 2) - (extent.x / 2), 5);

    Fuse::Get().GetRenderer().PushScreenQuad(position, extent, fill_color);
    Fuse::Get().GetRenderer().PushScreenBorder(position, extent, border_color, 3.0f);

    text_x = position.x + 1;
    text_y = position.y + 2;

    item_view_count = (size_t)(extent.y / 12);

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
  }

  void PopulateSettings() {
    settings.clear();

    for (int i = 0; i < 8; ++i) {
      PopulateShip(i);
    }

    const ClientSettings& s = Fuse::Get().GetSettings();

#define PUSH_SETTING(type) settings.emplace_back("Bomb", #type, std::to_string(s.type))
    PUSH_SETTING(BBombDamagePercent);
    PUSH_SETTING(BombAliveTime);
    PUSH_SETTING(BombDamageLevel);
    PUSH_SETTING(BombExplodeDelay);
    PUSH_SETTING(BombExplodePixels);
    PUSH_SETTING(BombSafety);
    PUSH_SETTING(EBombDamagePercent);
    PUSH_SETTING(EBombShutdownTime);
    PUSH_SETTING(JitterTime);
    PUSH_SETTING(ProximityDistance);
#undef PUSH_SETTING

#define PUSH_SETTING(type) settings.emplace_back("Brick", #type, std::to_string(s.type))
    PUSH_SETTING(BrickTime);
#undef PUSH_SETTING

#define PUSH_SETTING(type) settings.emplace_back("Bullet", #type, std::to_string(s.type))
    PUSH_SETTING(BulletAliveTime);
    PUSH_SETTING(BulletDamageLevel);
    PUSH_SETTING(BulletDamageUpgrade);
    PUSH_SETTING(ExactDamage);
#undef PUSH_SETTING

    settings.emplace_back("Burst", "BurstDamageLevel", std::to_string(s.BurstDamageLevel));

#define PUSH_SETTING(type) settings.emplace_back("Door", #type, std::to_string(s.type))
    PUSH_SETTING(DoorDelay);
    PUSH_SETTING(DoorMode);
#undef PUSH_SETTING

#define PUSH_SETTING(type) settings.emplace_back("Flag", #type, std::to_string(s.type))
    PUSH_SETTING(CarryFlags);
    PUSH_SETTING(EnterGameFlaggingDelay);
    PUSH_SETTING(FlagBlankDelay);
    PUSH_SETTING(FlagDropDelay);
    PUSH_SETTING(FlagDropResetReward);
    PUSH_SETTING(FlaggerBombFireDelay);
    PUSH_SETTING(FlaggerBombUpgrade);
    PUSH_SETTING(FlaggerDamagePercent);
    PUSH_SETTING(FlaggerFireCostPercent);
    PUSH_SETTING(FlaggerGunUpgrade);
    PUSH_SETTING(FlaggerKillMultiplier);
    PUSH_SETTING(FlaggerOnRadar);
    PUSH_SETTING(FlaggerSpeedAdjustment);
    PUSH_SETTING(FlaggerThrustAdjustment);
    PUSH_SETTING(NoDataFlagDropDelay);
#undef PUSH_SETTING

#define PUSH_SETTING(type) settings.emplace_back("Kill", #type, std::to_string(s.type))
    PUSH_SETTING(BountyIncreaseForKill);
    PUSH_SETTING(EnterDelay);
    PUSH_SETTING(MaxBonus);
    PUSH_SETTING(MaxPenalty);
    PUSH_SETTING(RewardBase);
#undef PUSH_SETTING

#define PUSH_SETTING(type) settings.emplace_back("Latency", #type, std::to_string(s.type))
    PUSH_SETTING(ClientSlowPacketSampleSize);
    PUSH_SETTING(ClientSlowPacketTime);
    PUSH_SETTING(S2CNoDataKickoutDelay);
    PUSH_SETTING(SendRoutePercent);
#undef PUSH_SETTING

    settings.emplace_back("Message", "AllowAudioMessages", std::to_string(s.AllowAudioMessages));

#define PUSH_SETTING(type) settings.emplace_back("Mine", #type, std::to_string(s.type))
    PUSH_SETTING(MineAliveTime);
    PUSH_SETTING(TeamMaxMines);
#undef PUSH_SETTING

#define PUSH_SETTING(type) settings.emplace_back("Misc", #type, std::to_string(s.type))
    PUSH_SETTING(ActivateAppShutdownTime);
    PUSH_SETTING(AllowSavedShips);
    PUSH_SETTING(AntiwarpSettleDelay);
    PUSH_SETTING(BounceFactor);
    PUSH_SETTING(DecoyAliveTime);
    PUSH_SETTING(DisableScreenshot);
    PUSH_SETTING(ExtraPositionData);
    PUSH_SETTING(FrequencyShift);
    PUSH_SETTING(MaxTimerDrift);
    PUSH_SETTING(NearDeathLevel);
    PUSH_SETTING(SafetyLimit);
    PUSH_SETTING(SendPositionDelay);
    PUSH_SETTING(SlowFrameCheck);
    PUSH_SETTING(SlowFrameRate);
    PUSH_SETTING(TickerDelay);
    PUSH_SETTING(VictoryMusic);
    PUSH_SETTING(WarpPointDelay);
    PUSH_SETTING(WarpRadiusLimit);
#undef PUSH_SETTING

#define PUSH_SETTING(type) settings.emplace_back("Prize", #type, std::to_string(s.type))
    PUSH_SETTING(DeathPrizeTime);
    PUSH_SETTING(EngineShutdownTime);
    PUSH_SETTING(MinimumVirtual);
    PUSH_SETTING(MultiPrizeCount);
    PUSH_SETTING(PrizeDelay);
    PUSH_SETTING(PrizeFactor);
    PUSH_SETTING(PrizeHideCount);
    PUSH_SETTING(PrizeMaxExist);
    PUSH_SETTING(PrizeMinExist);
    PUSH_SETTING(PrizeNegativeFactor);
    PUSH_SETTING(TakePrizeReliable);
    PUSH_SETTING(UpgradeVirtual);
#undef PUSH_SETTING

#define PUSH_SETTING(type) settings.emplace_back("PrizeWeight", #type, std::to_string(s.PrizeWeights.type))
    PUSH_SETTING(AllWeapons);
    PUSH_SETTING(AntiWarp);
    PUSH_SETTING(Bomb);
    PUSH_SETTING(BouncingBullets);
    PUSH_SETTING(Brick);
    PUSH_SETTING(Burst);
    PUSH_SETTING(Cloak);
    PUSH_SETTING(Decoy);
    PUSH_SETTING(Energy);
    PUSH_SETTING(Glue);
    PUSH_SETTING(Gun);
    PUSH_SETTING(MultiFire);
    PUSH_SETTING(MultiPrize);
    PUSH_SETTING(Portal);
    PUSH_SETTING(Proximity);
    PUSH_SETTING(QuickCharge);
    PUSH_SETTING(Recharge);
    PUSH_SETTING(Repel);
    PUSH_SETTING(Rocket);
    PUSH_SETTING(Rotation);
    PUSH_SETTING(Shields);
    PUSH_SETTING(Shrapnel);
    PUSH_SETTING(Stealth);
    PUSH_SETTING(Thor);
    PUSH_SETTING(Thruster);
    PUSH_SETTING(TopSpeed);
    PUSH_SETTING(Warp);
    PUSH_SETTING(XRadar);
#undef PUSH_SETTING

#define PUSH_SETTING(type) settings.emplace_back("Radar", #type, std::to_string(s.type))
    PUSH_SETTING(MapZoomFactor);
    PUSH_SETTING(RadarMode);
    PUSH_SETTING(RadarNeutralSize);
#undef PUSH_SETTING

#define PUSH_SETTING(type) settings.emplace_back("Repel", #type, std::to_string(s.type))
    PUSH_SETTING(RepelDistance);
    PUSH_SETTING(RepelSpeed);
    PUSH_SETTING(RepelTime);
#undef PUSH_SETTING

    settings.emplace_back("Rocket", "RocketSpeed", std::to_string(s.RocketSpeed));
    settings.emplace_back("Rocket", "RocketThrust", std::to_string(s.RocketThrust));

#define PUSH_SETTING(type) settings.emplace_back("Shrapnel", #type, std::to_string(s.type))
    PUSH_SETTING(InactiveShrapDamage);
    PUSH_SETTING(ShrapnelDamagePercent);
    PUSH_SETTING(ShrapnelRandom);
    PUSH_SETTING(ShrapnelSpeed);
#undef PUSH_SETTING

#define PUSH_SETTING(type) settings.emplace_back("Soccer", #type, std::to_string(s.type))
    PUSH_SETTING(AllowBombs);
    PUSH_SETTING(AllowGuns);
    PUSH_SETTING(BallBlankDelay);
    PUSH_SETTING(BallBounce);
    PUSH_SETTING(BallLocation);
    PUSH_SETTING(DisableBallKilling);
    PUSH_SETTING(DisableBallThroughWalls);
    PUSH_SETTING(PassDelay);
    PUSH_SETTING(SoccerMode);
    PUSH_SETTING(UseFlagger);
#undef PUSH_SETTING

#define PUSH_SETTING(type) settings.emplace_back("Spawn", "Team0-" #type, std::to_string(s.SpawnSettings[0].type))
    PUSH_SETTING(Radius);
    PUSH_SETTING(X);
    PUSH_SETTING(Y);
#undef PUSH_SETTING

#define PUSH_SETTING(type) settings.emplace_back("Spawn", "Team1-" #type, std::to_string(s.SpawnSettings[1].type))
    PUSH_SETTING(Radius);
    PUSH_SETTING(X);
    PUSH_SETTING(Y);
#undef PUSH_SETTING

#define PUSH_SETTING(type) settings.emplace_back("Spawn", "Team2-" #type, std::to_string(s.SpawnSettings[2].type))
    PUSH_SETTING(Radius);
    PUSH_SETTING(X);
    PUSH_SETTING(Y);
#undef PUSH_SETTING

#define PUSH_SETTING(type) settings.emplace_back("Spawn", "Team3-" #type, std::to_string(s.SpawnSettings[3].type))
    PUSH_SETTING(Radius);
    PUSH_SETTING(X);
    PUSH_SETTING(Y);
#undef PUSH_SETTING

#define PUSH_SETTING(type) settings.emplace_back("Spectator", #type, std::to_string(s.type))
    PUSH_SETTING(HideFlags);
    PUSH_SETTING(NoXRadar);
#undef PUSH_SETTING

#define PUSH_SETTING(type) settings.emplace_back("Team", #type, std::to_string(s.type))
    PUSH_SETTING(MaxFrequency);
    PUSH_SETTING(MaxPerPrivateTeam);
    PUSH_SETTING(MaxPerTeam);
#undef PUSH_SETTING

    settings.emplace_back("Toggle", "AntiWarpPixels", std::to_string(s.AntiWarpPixels));

#define PUSH_SETTING(type) settings.emplace_back("Wormhole", #type, std::to_string(s.type))
    PUSH_SETTING(GravityBombs);
    PUSH_SETTING(WormholeSwitchTime);
#undef PUSH_SETTING
  }
  void PopulateShip(int ship) {
    const char* kShipNames[] = {"Warbird", "Javelin", "Spider", "Leviathan", "Terrier", "Weasel", "Lancaster", "Shark"};

    const ShipSettings& s = Fuse::Get().GetShipSettings(ship);

    std::string_view name = kShipNames[ship];

#define PUSH_SETTING(type) settings.emplace_back(name, #type, std::to_string(s.type))
    PUSH_SETTING(SuperTime);
    PUSH_SETTING(ShieldsTime);
    PUSH_SETTING(Gravity);
    PUSH_SETTING(GravityTopSpeed);
    PUSH_SETTING(BulletFireEnergy);
    PUSH_SETTING(MultiFireEnergy);
    PUSH_SETTING(BombFireEnergy);
    PUSH_SETTING(LandmineFireEnergy);
    PUSH_SETTING(LandmineFireEnergyUpgrade);
    PUSH_SETTING(BulletSpeed);
    PUSH_SETTING(BombSpeed);
    PUSH_SETTING(SeeBombLevel);
    PUSH_SETTING(DisableFastShooting);
    PUSH_SETTING(Radius);
    PUSH_SETTING(MultiFireAngle);
    PUSH_SETTING(CloakEnergy);
    PUSH_SETTING(StealthEnergy);
    PUSH_SETTING(AntiWarpEnergy);
    PUSH_SETTING(XRadarEnergy);
    PUSH_SETTING(MaximumRotation);
    PUSH_SETTING(MaximumThrust);
    PUSH_SETTING(MaximumSpeed);
    PUSH_SETTING(MaximumRecharge);
    PUSH_SETTING(MaximumEnergy);
    PUSH_SETTING(InitialRotation);
    PUSH_SETTING(InitialThrust);
    PUSH_SETTING(InitialSpeed);
    PUSH_SETTING(InitialRecharge);
    PUSH_SETTING(InitialEnergy);
    PUSH_SETTING(UpgradeRotation);
    PUSH_SETTING(UpgradeThrust);
    PUSH_SETTING(UpgradeSpeed);
    PUSH_SETTING(UpgradeRecharge);
    PUSH_SETTING(UpgradeEnergy);
    PUSH_SETTING(AfterburnerEnergy);
    PUSH_SETTING(BombThrust);
    PUSH_SETTING(BurstSpeed);
    PUSH_SETTING(TurretThrustPenalty);
    PUSH_SETTING(BulletFireDelay);
    PUSH_SETTING(MultiFireDelay);
    PUSH_SETTING(BombFireDelay);
    PUSH_SETTING(LandmineFireDelay);
    PUSH_SETTING(RocketTime);
    PUSH_SETTING(InitialBounty);
    PUSH_SETTING(DamageFactor);
    PUSH_SETTING(PrizeShareLimit);
    PUSH_SETTING(AttachBounty);
    PUSH_SETTING(SoccerBallThrowTimer);
    PUSH_SETTING(SoccerBallFriction);
    PUSH_SETTING(SoccerBallSpeed);
    PUSH_SETTING(TurretLimit);
    PUSH_SETTING(BurstShrapnel);
    PUSH_SETTING(MaxMines);
    PUSH_SETTING(RepelMax);
    PUSH_SETTING(BurstMax);
    PUSH_SETTING(DecoyMax);
    PUSH_SETTING(ThorMax);
    PUSH_SETTING(BrickMax);
    PUSH_SETTING(RocketMax);
    PUSH_SETTING(PortalMax);
    PUSH_SETTING(InitialRepel);
    PUSH_SETTING(InitialBurst);
    PUSH_SETTING(InitialBrick);
    PUSH_SETTING(InitialRocket);
    PUSH_SETTING(InitialThor);
    PUSH_SETTING(InitialDecoy);
    PUSH_SETTING(InitialPortal);
    PUSH_SETTING(BombBounceCount);
    PUSH_SETTING(ShrapnelMax);
    PUSH_SETTING(ShrapnelRate);
    PUSH_SETTING(CloakStatus);
    PUSH_SETTING(StealthStatus);
    PUSH_SETTING(XRadarStatus);
    PUSH_SETTING(AntiWarpStatus);
    PUSH_SETTING(InitialGuns);
    PUSH_SETTING(MaxGuns);
    PUSH_SETTING(InitialBombs);
    PUSH_SETTING(MaxBombs);
    PUSH_SETTING(DoubleBarrel);
    PUSH_SETTING(EmpBomb);
    PUSH_SETTING(SeeMines);
#undef PUSH_SETTING
  }

  Vector2f surface_size;
  Vector2f extent;

  float text_x;
  float text_y;

  bool open = false;
  size_t selected_index = 0;
  size_t view_index = 0;
  size_t item_view_count = 25;

  std::vector<Setting> settings;
};

class SettingsHook final : public HookInjection {
 public:
  const char* GetHookName() override { return "Settings"; }

  void OnUpdate() override {
    auto self = Fuse::Get().GetPlayer();
    if (!self) return;

    window.Render();
  }

  KeyState OnGetAsyncKeyState(int vKey) override { return {}; }

  void OnWindowsEvent(MSG msg, WPARAM wParam, LPARAM lParam) override {
    switch (msg.message) {
      case WM_KEYDOWN: {
        if (wParam == VK_F12 && Fuse::Get().IsGameMenuOpen()) {
          window.Toggle();
        }

        if (window.open) {
          switch (wParam) {
            case VK_ESCAPE: {
              window.Toggle();
              // Set game menu open so it closes when Continuum handles the escape key.
              Fuse::Get().SetGameMenuOpen(true);
            } break;
            case VK_NEXT: {
              if (GetAsyncKeyState(VK_SHIFT)) {
                window.selected_index += window.item_view_count;
              } else {
                ++window.selected_index;
              }

              if (window.selected_index >= window.settings.size()) {
                window.selected_index = window.settings.size() - 1;
              }
            } break;
            case VK_PRIOR: {
              if (GetAsyncKeyState(VK_SHIFT)) {
                window.selected_index -= window.item_view_count;
              } else {
                --window.selected_index;
              }

              if (window.selected_index >= window.settings.size()) {
                window.selected_index = 0;
              }
            } break;
            default: {
            } break;
          }
        }

      } break;
    }
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
