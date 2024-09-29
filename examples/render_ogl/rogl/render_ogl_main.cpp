#include <fuse/Fuse.h>
#include <rogl/Platform.h>
#include <rogl/ddraw/IDirectDraw.h>

#include <format>
#include <string>
#include <string_view>

//
#include <detours.h>

using namespace fuse;
using namespace rogl;

static HRESULT(WINAPI* RealDirectDrawCreate)(GUID* lpGUID, LPDIRECTDRAW* lplpDD,
                                             IUnknown* pUnkOuter) = DirectDrawCreate;
static int(WINAPI* RealSetDIBitsToDevice)(_In_ HDC hdc, _In_ int xDest, _In_ int yDest, _In_ DWORD w, _In_ DWORD h,
                                          _In_ int xSrc, _In_ int ySrc, _In_ UINT StartScan, _In_ UINT cLines,
                                          _In_ CONST VOID* lpvBits, _In_ CONST BITMAPINFO* lpbmi,
                                          _In_ UINT ColorUse) = SetDIBitsToDevice;
static ATOM(WINAPI* RealRegisterClassA)(const WNDCLASSA* wc) = RegisterClassA;
static BOOL(WINAPI* RealDestroyWindow)(HWND hwnd) = DestroyWindow;

HRESULT __stdcall OverrideDirectDrawCreate(GUID* lpGUID, LPDIRECTDRAW* lplpDD, IUnknown* pUnkOuter) {
  *lplpDD = OglDirectDrawCreate();
  return S_OK;
}

// hdc stores the opengl texture id by using the direct draw surface override.
int WINAPI OverrideSetDIBitsToDevice(_In_ HDC hdc, _In_ int xDest, _In_ int yDest, _In_ DWORD w, _In_ DWORD h,
                                     _In_ int xSrc, _In_ int ySrc, _In_ UINT StartScan, _In_ UINT cLines,
                                     _In_ CONST VOID* lpvBits, _In_ CONST BITMAPINFO* lpbmi, _In_ UINT ColorUse) {
  GLuint tex_id = (GLuint)hdc;

  // If we have low bit count, then we are palettized.
  if (lpbmi->bmiHeader.biBitCount <= 8) {
    u32 colors_used = 1 << lpbmi->bmiHeader.biBitCount;

    // TODO: Generic malloc not necessary here. Use bump allocator.
    u8* data = (u8*)malloc(w * h * 4);

    if (!data) {
      DisplayMessage("Failed to allocate cpu buffer for uploading texture.");
      exit(1);
      return cLines;
    }

    u8* palette_indices = (u8*)lpvBits;

    for (size_t i = 0; i < w * h; ++i) {
      RGBQUAD color = lpbmi->bmiColors[palette_indices[i]];
      data[i * 4 + 0] = color.rgbBlue;
      data[i * 4 + 1] = color.rgbGreen;
      data[i * 4 + 2] = color.rgbRed;
      data[i * 4 + 3] = 0xFF;
    }

    OglRenderer::Get().UploadTexture(tex_id, w, h, data, w * h * 4);

    free(data);
  } else if (lpbmi->bmiHeader.biBitCount == 24) {
    // TODO: Generic malloc not necessary here. Use bump allocator.
    u8* data = (u8*)malloc(w * h * 4);

    if (!data) {
      DisplayMessage("Failed to allocate cpu buffer for uploading texture.");
      exit(1);
      return cLines;
    }

    u8* odata = (u8*)lpvBits;

    for (size_t i = 0; i < w * h; ++i) {
      data[i * 4 + 0] = odata[i * 3 + 0];
      data[i * 4 + 1] = odata[i * 3 + 1];
      data[i * 4 + 2] = odata[i * 3 + 2];
      data[i * 4 + 3] = 0xFF;
    }

    OglRenderer::Get().UploadTexture(tex_id, w, h, data, w * h * 4);

    free(data);
  } else if (lpbmi->bmiHeader.biBitCount == 32) {
    OglRenderer::Get().UploadTexture(tex_id, w, h, (u8*)lpvBits, w * h * 4);
  } else {
    DisplayMessage("Parsing bitmap with bit count of {} not implemented.", lpbmi->bmiHeader.biBitCount);
    exit(1);
  }

  return cLines;
}

ATOM WINAPI OverrideRegisterClassA(const WNDCLASS* wc) {
  WNDCLASS custom_wc = *wc;

  // Override the style to enable owned dc for opengl.
  custom_wc.style |= CS_OWNDC;

  return RealRegisterClassA(&custom_wc);
}

BOOL WINAPI OverrideDestroyWindow(HWND hwnd) {
  HWND game_window = Fuse::Get().GetGameWindowHandle();

  if (hwnd == game_window) {
    OglRenderer::Get().DestroyContext();
  }

  return RealDestroyWindow(hwnd);
}

class RenderOglHook final : public HookInjection {
 public:
  const char* GetHookName() override { return "RenderOgl"; }

  RenderOglHook() { Inject(); }

  void OnUpdate() override { OglRenderer::Get().Render(); }

  KeyState OnGetAsyncKeyState(int vKey) override { return {}; }

  void Inject() {
    DetourRestoreAfterWith();

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)RealDirectDrawCreate, OverrideDirectDrawCreate);
    DetourAttach(&(PVOID&)RealSetDIBitsToDevice, OverrideSetDIBitsToDevice);
    DetourAttach(&(PVOID&)RealRegisterClassA, OverrideRegisterClassA);
    DetourAttach(&(PVOID&)RealDestroyWindow, OverrideDestroyWindow);
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
