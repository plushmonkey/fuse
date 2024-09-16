#include <fuse/Fuse.h>
#include <fuse/HookInjection.h>

using namespace fuse;

class BoundingBoxHook final : public HookInjection {
 public:
  const char* GetHookName() override { return "BoundingBox"; }

  void OnUpdate() override {
    auto self = Fuse::Get().GetPlayer();
    if (!self) return;

    for (auto& player : Fuse::Get().GetPlayers()) {
      auto bounds_color = self->frequency == player.frequency ? fuse::render::Color::FromRGB(0, 255, 0)
                                                              : fuse::render::Color::FromRGB(255, 0, 0);

      RenderPlayerBounds(player, bounds_color);
    }
  }

  void RenderPlayerBounds(const Player& player, fuse::render::Color bounds_color) {
    if (player.ship >= 8) return;

#if 0
    float radius = Fuse::Get().GetShipSettings(player.ship).GetRadius();
#else
    float radius = Fuse::Get().GetShipSettings(player.ship).SoccerBallProximity / 16.0f;
#endif

    // TODO: The renderer needs to be improved to smooth out the rendering of the box.
    // It wiggles around too much to actually be used.

    Vector2f bounds_tl = player.position + Vector2f(-radius, -radius);
    Vector2f bounds_tr = player.position + Vector2f(radius, -radius);
    Vector2f bounds_br = player.position + Vector2f(radius, radius);
    Vector2f bounds_bl = player.position + Vector2f(-radius, radius);

    Fuse::Get().GetRenderer().PushWorldLine(bounds_tl, bounds_tr, bounds_color);
    Fuse::Get().GetRenderer().PushWorldLine(bounds_tl, bounds_bl, bounds_color);
    Fuse::Get().GetRenderer().PushWorldLine(bounds_tr, bounds_br, bounds_color);
    Fuse::Get().GetRenderer().PushWorldLine(bounds_bl, bounds_br, bounds_color);

    Fuse::Get().GetRenderer().PushWorldLine(player.position, player.position + player.GetHeading() * 10.0f,
                                            bounds_color);
  }

  KeyState OnGetAsyncKeyState(int vKey) override { return {}; }
};

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID reserved) {
  switch (dwReason) {
    case DLL_PROCESS_ATTACH: {
      Fuse::Get().RegisterHook(std::make_unique<BoundingBoxHook>());
    } break;
  }
  return TRUE;
}
