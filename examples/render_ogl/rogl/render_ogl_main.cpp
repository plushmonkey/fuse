#include <fuse/Fuse.h>
#include <rogl/Platform.h>
#include <rogl/ddraw/IDirectDraw.h>
#include <rogl/ddraw/IDirectDrawSurface.h>

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

#if 0
static void SaveBitmap(int width, int height, u8* data, size_t size) {
#pragma pack(push, 1)
  struct bitmap_file_header {
    u16 identifier;
    u32 fileSize;
    u16 reserved1;
    u16 reserved2;
    u32 startOffset;
  };

  struct bitmap_dib_header {
    u32 headerSize;
    s32 width;
    s32 height;
    u16 planes;
    u16 bitsPerPixel;
    u32 compression;
    u32 imageSize;
    u32 resolutionHorizontal;
    u32 resolutionVertical;
    u32 colorCount;
    u32 importantColors;
  };
#pragma pack(pop)

  bitmap_file_header m_FileHeader = {};
  bitmap_dib_header m_InfoHeader = {};

  m_FileHeader.identifier = 0x4D42;
  m_FileHeader.startOffset = sizeof(bitmap_file_header) + sizeof(bitmap_dib_header);
  m_FileHeader.fileSize = m_FileHeader.startOffset + size;

  m_InfoHeader.headerSize = sizeof(bitmap_dib_header);
  m_InfoHeader.width = width;
  m_InfoHeader.height = height;
  m_InfoHeader.planes = 1;
  m_InfoHeader.bitsPerPixel = 32;
  m_InfoHeader.compression = 0;
  m_InfoHeader.imageSize = size;

  FILE* f = fopen("test.bmp", "wb");
  fwrite(&m_FileHeader, sizeof(m_FileHeader), 1, f);
  fwrite(&m_InfoHeader, sizeof(m_InfoHeader), 1, f);
  fwrite(data, size, 1, f);
  fclose(f);
}
#endif

// hdc stores the surface by using the direct draw surface override of GetDC.
int WINAPI OverrideSetDIBitsToDevice(_In_ HDC hdc, _In_ int xDest, _In_ int yDest, _In_ DWORD w, _In_ DWORD h,
                                     _In_ int xSrc, _In_ int ySrc, _In_ UINT StartScan, _In_ UINT cLines,
                                     _In_ CONST VOID* lpvBits, _In_ CONST BITMAPINFO* lpbmi, _In_ UINT ColorUse) {
  OglDirectDrawSurface* surface = (OglDirectDrawSurface*)hdc;

  if (xDest != 0 || yDest != 0) {
    Fatal("Need to implement dest");
  }

  if (StartScan != 0) {
    Fatal("Need to implement StartScan");
  }

  if (!lpvBits || !lpbmi || lpbmi->bmiHeader.biBitCount == 0) {
    Fatal("Bad bitmap attempted to be uploaded to gpu.");
    return cLines;
  }

  if (lpbmi->bmiHeader.biWidth < 0 || lpbmi->bmiHeader.biHeight < 0) {
    Fatal("Negative");
  }

  if (lpbmi->bmiHeader.biCompression != 0) {
    Fatal("Not zero compression.");
  }

  // If we have low bit count, then we are palettized.
  if (lpbmi->bmiHeader.biBitCount == 4) {
    u8* data = (u8*)malloc(w * h * 4);

    if (!data) {
      Fatal("Failed to allocate cpu buffer for uploading texture.");
      return cLines;
    }

    size_t offset = (ySrc * w + xSrc) / 2;
    u8* palette_indices = (u8*)lpvBits + offset;

    for (u32 y = 0; y < h; ++y) {
      for (u32 x = 0; x < w; ++x) {
        size_t index = (y * w + x) * 4;
        size_t total_index = ((h - 1 - y) * w) + x;

        size_t p_index = 0;

        if (total_index & 1) {
          p_index = palette_indices[total_index / 2] & 0x0F;
        } else {
          p_index = (palette_indices[total_index / 2] & 0xF0) >> 4;
        }

        RGBQUAD color = lpbmi->bmiColors[p_index];

        data[index + 0] = color.rgbRed;
        data[index + 1] = color.rgbGreen;
        data[index + 2] = color.rgbBlue;
        data[index + 3] = 0xFF;
      }
    }

    OglRenderer::Get().UploadTexture(*surface, data, w * h * 4);

    free(data);
  } else if (lpbmi->bmiHeader.biBitCount == 8) {
    u8* data = (u8*)malloc(w * h * 4);

    if (!data) {
      Fatal("Failed to allocate cpu buffer for uploading texture.");
      return cLines;
    }

    size_t offset = ySrc * w + xSrc;
    u8* palette_indices = (u8*)lpvBits + offset;

    // Align to DWORD as required by SetDIBitsToDevice.
    size_t scanline_size = (w + 3) & ~3;

    for (u32 y = 0; y < h; ++y) {
      for (u32 x = 0; x < w; ++x) {
        size_t index = (y * w + x) * 4;
        u8* row = palette_indices + (h - 1 - y) * scanline_size;
        size_t p_index = row[x];

        RGBQUAD color = lpbmi->bmiColors[p_index];

        data[index + 0] = color.rgbRed;
        data[index + 1] = color.rgbGreen;
        data[index + 2] = color.rgbBlue;
        data[index + 3] = 0xFF;
      }
    }

    OglRenderer::Get().UploadTexture(*surface, data, w * h * 4);

    free(data);
  } else if (lpbmi->bmiHeader.biBitCount == 24) {
    u8* data = (u8*)malloc(w * h * 4);

    if (!data) {
      Fatal("Failed to allocate cpu buffer for uploading texture.");
      return cLines;
    }

    size_t offset = ySrc * w + xSrc;
    u8* odata = (u8*)lpvBits + offset * 3;

    for (u32 y = 0; y < h; ++y) {
      for (u32 x = 0; x < w; ++x) {
        size_t index = (y * w + x) * 4;
        size_t o_index = ((h - 1 - y) * w + x) * 3;

        data[index + 0] = odata[o_index + 0];
        data[index + 1] = odata[o_index + 1];
        data[index + 2] = odata[o_index + 2];
        data[index + 3] = 0xFF;
      }
    }

    OglRenderer::Get().UploadTexture(*surface, 0, w * h * 4);

    free(data);
  } else if (lpbmi->bmiHeader.biBitCount == 32) {
    size_t offset = ySrc * w + xSrc;
    u8* data = (u8*)malloc(w * h * 4);
    u8* odata = (u8*)lpvBits;

    if (!data) {
      Fatal("Failed to allocate cpu buffer for uploading texture.");
      return cLines;
    }

    for (u32 y = 0; y < h; ++y) {
      for (u32 x = 0; x < w; ++x) {
        size_t index = (y * w + x) * 4;
        size_t o_index = ((h - 1 - y) * w + x) * 4;

        data[index + 0] = odata[o_index + 0];
        data[index + 1] = odata[o_index + 1];
        data[index + 2] = odata[o_index + 2];
        data[index + 3] = odata[o_index + 3];
      }
    }

    OglRenderer::Get().UploadTexture(*surface, (u8*)data + offset * 4, w * h * 4);
  } else {
    Fatal("Parsing bitmap with bit count of {} not implemented.", lpbmi->bmiHeader.biBitCount);
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

  void OnUpdate() override {}

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
