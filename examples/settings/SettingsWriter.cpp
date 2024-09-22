#include "SettingsWriter.h"

void SettingsWriter::WriteShip(const fuse::ClientSettings& settings, int ship) {
  static const char* kShipNames[] = {"Warbird", "Javelin", "Spider",    "Leviathan",
                                     "Terrier", "Weasel",  "Lancaster", "Shark"};

  const fuse::ShipSettings& s = settings.ShipSettings[ship];

  std::string_view name = kShipNames[ship];

  WriteHeader(name);

#define PUSH_SETTING(type) WriteSetting(#type, std::to_string(s.type));
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

bool SettingsWriter::Write(const fuse::ClientSettings& s) {
  if (!WriteBegin()) return false;

  for (int i = 0; i < 8; ++i) {
    WriteShip(s, i);
  }

#define PUSH_SETTING(type) WriteSetting(#type, std::to_string(s.type))

  WriteHeader("Bomb");
  PUSH_SETTING(BBombDamagePercent);
  PUSH_SETTING(BombAliveTime);
  WriteSetting("BombDamageLevel", std::to_string(s.BombDamageLevel / 1000));
  PUSH_SETTING(BombExplodeDelay);
  PUSH_SETTING(BombExplodePixels);
  PUSH_SETTING(BombSafety);
  PUSH_SETTING(EBombDamagePercent);
  PUSH_SETTING(EBombShutdownTime);
  PUSH_SETTING(JitterTime);
  PUSH_SETTING(ProximityDistance);

  WriteHeader("Brick");
  PUSH_SETTING(BrickTime);

  WriteHeader("Bullet");
  PUSH_SETTING(BulletAliveTime);
  WriteSetting("BulletDamageLevel", std::to_string(s.BulletDamageLevel / 1000));
  WriteSetting("BulletDamageUpgrade", std::to_string(s.BulletDamageUpgrade / 1000));
  PUSH_SETTING(ExactDamage);

  WriteHeader("Burst");
  WriteSetting("BurstDamageLevel", std::to_string(s.BurstDamageLevel / 1000));

  WriteHeader("Door");
  PUSH_SETTING(DoorDelay);
  PUSH_SETTING(DoorMode);

  WriteHeader("Flag");
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

  WriteHeader("Kill");
  PUSH_SETTING(BountyIncreaseForKill);
  PUSH_SETTING(EnterDelay);
  PUSH_SETTING(MaxBonus);
  PUSH_SETTING(MaxPenalty);
  PUSH_SETTING(RewardBase);

  WriteHeader("Latency");
  PUSH_SETTING(ClientSlowPacketSampleSize);
  PUSH_SETTING(ClientSlowPacketTime);
  PUSH_SETTING(S2CNoDataKickoutDelay);
  PUSH_SETTING(SendRoutePercent);

  WriteHeader("Message");
  PUSH_SETTING(AllowAudioMessages);
  PUSH_SETTING(MessageReliable);

  WriteHeader("Mine");
  PUSH_SETTING(MineAliveTime);
  PUSH_SETTING(TeamMaxMines);

  WriteHeader("Misc");
  PUSH_SETTING(ActivateAppShutdownTime);
  PUSH_SETTING(AllowSavedShips);
  PUSH_SETTING(AntiwarpSettleDelay);
  WriteSetting("AntiWarpSettleDelay", std::to_string(s.AntiwarpSettleDelay));
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

  WriteHeader("Prize");
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

#define PUSH_SETTING(type) WriteSetting(#type, std::to_string(s.PrizeWeights.type))
  WriteHeader("PrizeWeight");
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

#define PUSH_SETTING(type) WriteSetting(#type, std::to_string(s.type))
  WriteHeader("Radar");
  PUSH_SETTING(MapZoomFactor);
  PUSH_SETTING(RadarMode);
  PUSH_SETTING(RadarNeutralSize);

  WriteHeader("Repel");
  PUSH_SETTING(RepelDistance);
  PUSH_SETTING(RepelSpeed);
  PUSH_SETTING(RepelTime);

  WriteHeader("Rocket");
  PUSH_SETTING(RocketSpeed);
  PUSH_SETTING(RocketThrust);

  WriteHeader("Shrapnel");
  WriteSetting("InactiveShrapDamage", std::to_string(s.InactiveShrapDamage / 1000));
  PUSH_SETTING(ShrapnelDamagePercent);
  WriteSetting("Random", std::to_string(s.ShrapnelRandom));
  PUSH_SETTING(ShrapnelSpeed);

  WriteHeader("Soccer");
  PUSH_SETTING(AllowBombs);
  PUSH_SETTING(AllowGuns);
  PUSH_SETTING(BallBlankDelay);
  PUSH_SETTING(BallBounce);
  PUSH_SETTING(BallLocation);
  PUSH_SETTING(DisableBallKilling);
  WriteSetting("DisableWallPass", std::to_string(s.DisableBallThroughWalls));
  PUSH_SETTING(PassDelay);
  WriteSetting("Mode", std::to_string(s.SoccerMode));
  PUSH_SETTING(UseFlagger);

  WriteHeader("Spawn");
  WriteSetting("Team0-Radius", std::to_string(s.SpawnSettings[0].Radius));
  WriteSetting("Team0-X", std::to_string(s.SpawnSettings[0].X));
  WriteSetting("Team0-Y", std::to_string(s.SpawnSettings[0].Y));
  WriteSetting("Team1-Radius", std::to_string(s.SpawnSettings[1].Radius));
  WriteSetting("Team1-X", std::to_string(s.SpawnSettings[1].X));
  WriteSetting("Team1-Y", std::to_string(s.SpawnSettings[1].Y));
  WriteSetting("Team2-Radius", std::to_string(s.SpawnSettings[2].Radius));
  WriteSetting("Team2-X", std::to_string(s.SpawnSettings[2].X));
  WriteSetting("Team2-Y", std::to_string(s.SpawnSettings[2].Y));
  WriteSetting("Team3-Radius", std::to_string(s.SpawnSettings[3].Radius));
  WriteSetting("Team3-X", std::to_string(s.SpawnSettings[3].X));
  WriteSetting("Team3-Y", std::to_string(s.SpawnSettings[3].Y));

  WriteHeader("Spectator");
  PUSH_SETTING(HideFlags);
  PUSH_SETTING(NoXRadar);

  WriteHeader("Team");
  PUSH_SETTING(MaxFrequency);
  PUSH_SETTING(MaxPerPrivateTeam);
  PUSH_SETTING(MaxPerTeam);

  WriteHeader("Toggle");
  PUSH_SETTING(AntiWarpPixels);

  WriteHeader("Wormhole");
  PUSH_SETTING(GravityBombs);
  WriteSetting("SwitchTime", std::to_string(s.WormholeSwitchTime));

  return WriteEnd();
}

FileSettingsWriter::FileSettingsWriter(std::string_view filename) : filename(filename) {}

bool FileSettingsWriter::WriteBegin() {
  std::string path(filename);

  f = fopen(path.data(), "wb");

  return f != nullptr;
}

bool FileSettingsWriter::WriteEnd() {
  if (f) {
    fclose(f);
    f = nullptr;
  }

  return true;
}

void FileSettingsWriter::WriteHeader(std::string_view category) {
  if (!f) return;

  char buffer[1024];

  if (ftell(f) != 0) {
    fwrite("\n", 1, 1, f);
  }

  size_t size = sprintf(buffer, "[%.*s]\n", (fuse::u32)category.size(), category.data());
  fwrite(buffer, 1, size, f);
}

void FileSettingsWriter::WriteSetting(std::string_view name, std::string_view value) {
  if (!f) return;

  char buffer[1024];

  size_t size =
      sprintf(buffer, "%.*s=%.*s\n", (fuse::u32)name.size(), name.data(), (fuse::u32)value.size(), value.data());

  fwrite(buffer, 1, size, f);
}
