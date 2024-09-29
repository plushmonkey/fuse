#ifndef CINTERFACE
#define CINTERFACE
#endif

#include "IDirectDraw.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <fuse/Fuse.h>
#include <memory.h>
#include <string.h>

#include <format>
#include <string>
#include <string_view>

#pragma comment(lib, "ddraw.lib")
#pragma comment(lib, "dxguid.lib")

#include "IDirectDrawPalette.h"
#include "IDirectDrawSurface.h"

#define INTERFACE OglDirectDraw

using namespace fuse;

static OglDirectDrawVtable vtable = {};

static void DisplayMessage(std::string_view msg) {
  MessageBox(NULL, msg.data(), "ogl_DirectDraw", MB_OK);
}

HRESULT __stdcall Ogl_QueryInterface(OglDirectDraw* This, REFIID riid, LPVOID FAR* ppvObj) {
  if (!*ppvObj) return E_INVALIDARG;

  if (!IsEqualGUID(riid, IID_IDirectDraw2) && !IsEqualGUID(riid, IID_IDirectDraw4) &&
      !IsEqualGUID(riid, IID_IDirectDraw7)) {
    return E_NOINTERFACE;
  }

  OglDirectDraw* ogl = (OglDirectDraw*)malloc(sizeof(OglDirectDraw));

  memcpy(&ogl->guid, &riid, sizeof(ogl->guid));
  ogl->lpVtbl = &vtable;
  ogl->ref = 1;

  *ppvObj = ogl;

  return S_OK;
}

ULONG __stdcall Ogl_AddRef(OglDirectDraw* This) {
  ULONG ret = ++This->ref;

  return ret;
}

ULONG __stdcall Ogl_Release(OglDirectDraw* This) {
  ULONG ret = --This->ref;

  if (ret == 0) {
    free(This);
  }

  return ret;
}

HRESULT __stdcall Ogl_Compact(OglDirectDraw* This) {
  DisplayMessage("Compact");
  return S_OK;
}

HRESULT __stdcall Ogl_CreateClipper(OglDirectDraw* This, DWORD, LPDIRECTDRAWCLIPPER FAR*, IUnknown FAR*) {
  DisplayMessage("CreateClipper");
  return S_OK;
}

HRESULT __stdcall Ogl_CreatePalette(OglDirectDraw* This, DWORD flags, LPPALETTEENTRY entry,
                                    LPDIRECTDRAWPALETTE FAR* palette, IUnknown FAR*) {
  //  DisplayMessage("CreatePalette");
  *palette = OglDirectDrawCreatePalette(entry);
  return S_OK;
}

HRESULT __stdcall Ogl_CreateSurface(OglDirectDraw* This, LPDDSURFACEDESC2 desc, LPDIRECTDRAWSURFACE7 FAR* surface,
                                    IUnknown FAR*) {
  if (!(desc->dwFlags & DDSD_HEIGHT)) {
    desc->dwHeight = 0;
  }

  if (!(desc->dwFlags & DDSD_WIDTH)) {
    desc->dwWidth = 0;
  }

  if (!(desc->dwFlags & DDSD_PITCH)) {
    desc->lPitch = 0;
  }

  if (!(desc->dwFlags & DDSD_BACKBUFFERCOUNT)) {
    desc->dwBackBufferCount = 0;
  }

  if (!(desc->dwFlags & DDSD_MIPMAPCOUNT)) {
    desc->dwMipMapCount = 0;
  }

  if (!(desc->dwFlags & DDSD_ALPHABITDEPTH)) {
    desc->dwAlphaBitDepth = 0;
  }

  if (!(desc->dwFlags & DDSD_LPSURFACE)) {
    desc->lpSurface = 0;
  }

  if (!(desc->dwFlags & DDSD_CKDESTOVERLAY)) {
    desc->ddckCKDestOverlay = {};
  }

  if (!(desc->dwFlags & DDSD_CKDESTBLT)) {
    desc->ddckCKDestBlt = {};
  }

  if (!(desc->dwFlags & DDSD_CKSRCOVERLAY)) {
    desc->ddckCKSrcOverlay = {};
  }

  if (!(desc->dwFlags & DDSD_CKSRCBLT)) {
    desc->ddckCKSrcBlt = {};
  }

  if (!(desc->dwFlags & DDSD_PIXELFORMAT)) {
    auto* pf = &desc->ddpfPixelFormat;
    *pf = {};
    pf->dwSize = sizeof(desc->ddpfPixelFormat);
    pf->dwFlags = DDPF_RGB;
    pf->dwRGBBitCount = 32;
    pf->dwRBitMask = 0xFF0000;
    pf->dwGBitMask = 0x00FF00;
    pf->dwBBitMask = 0x0000FF;
  }

  if (!(desc->dwFlags & DDSD_CAPS)) {
    desc->ddsCaps.dwCaps = 0;
  }

  if (desc->dwFlags & DDSD_CAPS) {
    u32 caps = desc->ddsCaps.dwCaps;

    if (caps & DDSCAPS_PRIMARYSURFACE) {
      // DisplayMessage(std::format("Creating primary surface."));
      *surface = OglDirectDrawCreateSurface(desc);
      OglRenderer::Get().CreateContext();
      return *surface ? S_OK : S_FALSE;
    } else {
      // DisplayMessage(std::format("Unknown caps: {}", desc->ddsCaps.dwCaps));
    }
  }

  // DisplayMessage(std::format("CreateSurface Flags: {}, Res: {}, {}, Mask: {}", desc->dwFlags, desc->dwWidth,
  // desc->dwHeight, desc->ddpfPixelFormat.dwRBitMask));

  *surface = OglDirectDrawCreateSurface(desc);

  return *surface ? S_OK : S_FALSE;
}

HRESULT __stdcall Ogl_DuplicateSurface(OglDirectDraw* This, LPDIRECTDRAWSURFACE7, LPDIRECTDRAWSURFACE7 FAR*) {
  DisplayMessage("DuplicateSurface");
  return S_OK;
}

HRESULT __stdcall Ogl_EnumDisplayModes(OglDirectDraw* This, DWORD flags, LPDDSURFACEDESC2 match_desc, LPVOID user,
                                       LPDDENUMMODESCALLBACK2 callback) {
  if (match_desc != nullptr) {
    DisplayMessage("EnumDisplayModes matching not implemented.");
    return S_FALSE;
  }

#if 0
  struct FormatSelection {
    u32 width;
    u32 height;
    u32 bits_per_pixel;
    u32 bytes_per_pixel;
    u32 value_5;
  };
#endif

  DDSURFACEDESC2 desc = {};

  desc.dwSize = sizeof(desc);
  desc.dwFlags = DDSD_HEIGHT | DDSD_WIDTH | DDSD_REFRESHRATE | DDSD_PIXELFORMAT;
  desc.dwWidth = 1920;
  desc.dwHeight = 1080;
  desc.dwRefreshRate = 60;

  auto* pf = &desc.ddpfPixelFormat;
  pf->dwSize = sizeof(desc.ddpfPixelFormat);
  pf->dwFlags = DDPF_RGB;
  pf->dwRGBBitCount = 32;
  pf->dwRBitMask = 0xFF0000;
  pf->dwGBitMask = 0x00FF00;
  pf->dwBBitMask = 0x0000FF;

  SIZE resolutions[] = {
      {624, 464},  {640, 480},   {720, 480},   {720, 576},   {800, 600},   {1024, 768},  {1152, 864},  {1176, 664},
      {1280, 720}, {1280, 768},  {1280, 800},  {1280, 960},  {1280, 1024}, {1360, 768},  {1366, 768},  {1440, 900},
      {1600, 900}, {1600, 1024}, {1600, 1200}, {1680, 1050}, {1920, 1080}, {1920, 1440}, {2048, 1536},
  };

  if (callback) {
    for (SIZE resolution : resolutions) {
      desc.dwWidth = resolution.cx;
      desc.dwHeight = resolution.cy;
      desc.lPitch = desc.dwWidth * (pf->dwRGBBitCount / 8);

      if (callback(&desc, user) == DDENUMRET_CANCEL) {
        return S_OK;
      }
    }
  }

  return S_OK;
}

HRESULT __stdcall Ogl_EnumSurfaces(THIS_ DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMSURFACESCALLBACK2) {
  DisplayMessage("EnumSurfaces");
  return S_OK;
}

HRESULT __stdcall Ogl_FlipToGDISurface(THIS) {
  DisplayMessage("FlipToGDISurface");
  return S_OK;
}

HRESULT __stdcall Ogl_GetCaps(THIS_ LPDDCAPS, LPDDCAPS) {
  DisplayMessage("GetCaps");
  return S_OK;
}

HRESULT __stdcall Ogl_GetDisplayMode(THIS_ LPDDSURFACEDESC2) {
  DisplayMessage("GetDisplayMode");
  return S_OK;
}

HRESULT __stdcall Ogl_GetFourCCCodes(THIS_ LPDWORD, LPDWORD) {
  DisplayMessage("GetFourCCCodes");
  return S_OK;
}

HRESULT __stdcall Ogl_GetGDISurface(THIS_ LPDIRECTDRAWSURFACE7 FAR*) {
  DisplayMessage("GetGDISurface");
  return S_OK;
}

HRESULT __stdcall Ogl_GetMonitorFrequency(THIS_ LPDWORD) {
  DisplayMessage("GetMonitorFrequency");
  return S_OK;
}

HRESULT __stdcall Ogl_GetScanLine(THIS_ LPDWORD) {
  DisplayMessage("GetScanLine");
  return S_OK;
}

HRESULT __stdcall Ogl_GetVerticalBlankStatus(THIS_ LPBOOL) {
  DisplayMessage("GetVerticalBlankStatus");
  return S_OK;
}

HRESULT __stdcall Ogl_Initialize(THIS_ GUID FAR*) {
  DisplayMessage("Initialize");
  return S_OK;
}

HRESULT __stdcall Ogl_RestoreDisplayMode(THIS) {
  DisplayMessage("RestoreDisplayMode");
  return S_OK;
}

HRESULT __stdcall Ogl_SetCooperativeLevel(THIS_ HWND, DWORD) {
  // DisplayMessage("SetCooperativeLevel");
  return S_OK;
}

HRESULT __stdcall Ogl_SetDisplayMode(THIS_ DWORD, DWORD, DWORD) {
  DisplayMessage("SetDisplayMode");
  return S_OK;
}

HRESULT __stdcall Ogl_WaitForVerticalBlank(THIS_ DWORD, HANDLE) {
  DisplayMessage("WaitForVerticalBlank");
  return S_OK;
}

HRESULT __stdcall Ogl_GetAvailableVidMem(THIS_ LPDDSCAPS2, LPDWORD, LPDWORD) {
  DisplayMessage("GetAvailableVidMem");
  return S_OK;
}

HRESULT __stdcall Ogl_GetSurfaceFromDC(THIS_ HDC, LPDIRECTDRAWSURFACE7*) {
  DisplayMessage("GetSurfaceFromDC");
  return S_OK;
}

HRESULT __stdcall Ogl_RestoreAllSurfaces(THIS) {
  DisplayMessage("RestoreAllSurfaces");
  return S_OK;
}

HRESULT __stdcall Ogl_TestCooperativeLevel(THIS) {
  DisplayMessage("TestCopperativeLevel");
  return S_OK;
}

HRESULT __stdcall Ogl_GetDeviceIdentifier(THIS_ LPDDDEVICEIDENTIFIER2, DWORD) {
  DisplayMessage("GetDeviceIdentifier");
  return S_OK;
}

HRESULT __stdcall Ogl_StartModeTest(THIS_ LPSIZE, DWORD, DWORD) {
  DisplayMessage("StartModeTest");
  return S_OK;
}

HRESULT __stdcall Ogl_EvaluateMode(THIS_ DWORD, DWORD*) {
  DisplayMessage("EvaluableMode");
  return S_OK;
}

IDirectDraw* __stdcall OglDirectDrawCreate() {
  vtable.QueryInterface = Ogl_QueryInterface;
  vtable.AddRef = Ogl_AddRef;
  vtable.Release = Ogl_Release;
  vtable.Compact = Ogl_Compact;
  vtable.CreateClipper = Ogl_CreateClipper;
  vtable.CreatePalette = Ogl_CreatePalette;
  vtable.CreateSurface = Ogl_CreateSurface;
  vtable.DuplicateSurface = Ogl_DuplicateSurface;
  vtable.EnumDisplayModes = Ogl_EnumDisplayModes;
  vtable.EnumSurfaces = Ogl_EnumSurfaces;
  vtable.FlipToGDISurface = Ogl_FlipToGDISurface;
  vtable.GetCaps = Ogl_GetCaps;
  vtable.GetDisplayMode = Ogl_GetDisplayMode;
  vtable.GetFourCCCodes = Ogl_GetFourCCCodes;
  vtable.GetGDISurface = Ogl_GetGDISurface;
  vtable.GetMonitorFrequency = Ogl_GetMonitorFrequency;
  vtable.GetScanLine = Ogl_GetScanLine;
  vtable.GetVerticalBlankStatus = Ogl_GetVerticalBlankStatus;
  vtable.Initialize = Ogl_Initialize;
  vtable.RestoreDisplayMode = Ogl_RestoreDisplayMode;
  vtable.SetCooperativeLevel = Ogl_SetCooperativeLevel;
  vtable.SetDisplayMode = Ogl_SetDisplayMode;
  vtable.WaitForVerticalBlank = Ogl_WaitForVerticalBlank;
  vtable.GetAvailableVidMem = Ogl_GetAvailableVidMem;
  vtable.GetSurfaceFromDC = Ogl_GetSurfaceFromDC;
  vtable.RestoreAllSurfaces = Ogl_RestoreAllSurfaces;
  vtable.TestCooperativeLevel = Ogl_TestCooperativeLevel;
  vtable.GetDeviceIdentifier = Ogl_GetDeviceIdentifier;
  vtable.StartModeTest = Ogl_StartModeTest;
  vtable.EvaluateMode = Ogl_EvaluateMode;

  OglDirectDraw* ogl = (OglDirectDraw*)malloc(sizeof(OglDirectDraw));

  memcpy(&ogl->guid, &IID_IDirectDraw7, sizeof(ogl->guid));
  ogl->lpVtbl = &vtable;
  IDirectDraw_AddRef(ogl);

  return (IDirectDraw*)ogl;
}
