#include <fuse/Fuse.h>

#include <format>
#include <string>
#include <string_view>

#include "IDirectDraw.h"

//
#include <detours.h>

using namespace fuse;

static HRESULT(WINAPI* RealDirectDrawCreate)(GUID* lpGUID, LPDIRECTDRAW* lplpDD,
                                             IUnknown* pUnkOuter) = DirectDrawCreate;
static int(WINAPI* RealSetDIBitsToDevice)(_In_ HDC hdc, _In_ int xDest, _In_ int yDest, _In_ DWORD w, _In_ DWORD h,
                                          _In_ int xSrc, _In_ int ySrc, _In_ UINT StartScan, _In_ UINT cLines,
                                          _In_ CONST VOID* lpvBits, _In_ CONST BITMAPINFO* lpbmi,
                                          _In_ UINT ColorUse) = SetDIBitsToDevice;
static ATOM(WINAPI* RealRegisterClassA)(const WNDCLASSA* wc) = RegisterClassA;

static void DisplayMessage(std::string_view msg) {
  MessageBox(NULL, msg.data(), "render_ogl", MB_OK);
}

HRESULT __stdcall OverrideDirectDrawCreate(GUID* lpGUID, LPDIRECTDRAW* lplpDD, IUnknown* pUnkOuter) {
  *lplpDD = OglDirectDrawCreate();
  return S_OK;
}

int WINAPI OverrideSetDIBitsToDevice(_In_ HDC hdc, _In_ int xDest, _In_ int yDest, _In_ DWORD w, _In_ DWORD h,
                                     _In_ int xSrc, _In_ int ySrc, _In_ UINT StartScan, _In_ UINT cLines,
                                     _In_ CONST VOID* lpvBits, _In_ CONST BITMAPINFO* lpbmi, _In_ UINT ColorUse) {
  // int result = RealSetDIBitsToDevice(hdc, xDest, yDest, w, h, xSrc, ySrc, StartScan, cLines, lpvBits, lpbmi,
  // ColorUse);
  return cLines;
}

ATOM WINAPI OverrideRegisterClassA(const WNDCLASS* wc) {
  WNDCLASS custom_wc = *wc;

  // Override the style to enable owned dc for opengl.
  custom_wc.style |= CS_OWNDC;

  return RealRegisterClassA(&custom_wc);
}

class RenderOglHook final : public HookInjection {
 public:
  const char* GetHookName() override { return "RenderOgl"; }

  RenderOglHook() { Inject(); }

  void OnUpdate() override {
    if (!glViewport) return;

    glViewport(0, 0, 1360, 768);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    SwapBuffers(GetDC(Fuse::Get().GetGameWindowHandle()));
  }

  KeyState OnGetAsyncKeyState(int vKey) override { return {}; }

  void Inject() {
    DetourRestoreAfterWith();

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)RealDirectDrawCreate, OverrideDirectDrawCreate);
    DetourAttach(&(PVOID&)RealSetDIBitsToDevice, OverrideSetDIBitsToDevice);
    DetourAttach(&(PVOID&)RealRegisterClassA, OverrideRegisterClassA);
    DetourTransactionCommit();
  }
};

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID reserved) {
  switch (dwReason) {
    case DLL_PROCESS_ATTACH: {
      Fuse::Get().RegisterHook(std::make_unique<RenderOglHook>());
    } break;
  }

  return TRUE;
}
