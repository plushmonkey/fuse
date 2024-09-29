#ifndef CINTERFACE
#define CINTERFACE
#endif

#include "IDirectDrawSurface.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <fuse/Fuse.h>
#include <memory.h>
#include <rogl/Platform.h>
#include <string.h>

#include <format>
#include <string>
#include <string_view>

#define INTERFACE OglDirectDrawSurface

using namespace fuse;
using namespace rogl;

static OglDirectDrawSurfaceVtable vtable = {};

// Chunk of memory to use for locked surfaces. This saves around 10MB of memory by having everything map here instead of
// malloc.
static char g_SurfaceBuffer[2048 * 2048 * 4];

HRESULT __stdcall OglSurface_QueryInterface(OglDirectDrawSurface* This, REFIID riid, LPVOID FAR* ppvObj) {
  if (!*ppvObj) return E_INVALIDARG;

  if (!IsEqualGUID(riid, IID_IDirectDrawSurface2) && !IsEqualGUID(riid, IID_IDirectDrawSurface4) &&
      !IsEqualGUID(riid, IID_IDirectDrawSurface7)) {
    return E_NOINTERFACE;
  }

  OglDirectDrawSurface* surface = (OglDirectDrawSurface*)malloc(sizeof(OglDirectDrawSurface));

  memcpy(&surface->guid, &riid, sizeof(surface->guid));
  surface->lpVtbl = &vtable;
  surface->ref = 1;

  *ppvObj = surface;

  return S_OK;
}

ULONG __stdcall OglSurface_AddRef(OglDirectDrawSurface* This) {
  ULONG ret = ++This->ref;

  return ret;
}

ULONG __stdcall OglSurface_Release(OglDirectDrawSurface* This) {
  ULONG ret = --This->ref;

  if (ret == 0) {
    free(This);
  }

  return ret;
}

HRESULT __stdcall OglSurface_AddAttachedSurface(OglDirectDrawSurface* This, LPDIRECTDRAWSURFACE7) {
  DisplayMessage("AddAttachedSurface");
  return S_OK;
}

HRESULT __stdcall OglSurface_AddOverlayDirtyRect(OglDirectDrawSurface* This, LPRECT) {
  DisplayMessage("AddOverlayDirtyRect");
  return S_OK;
}

HRESULT __stdcall OglSurface_Blt(OglDirectDrawSurface* This, LPRECT rect, LPDIRECTDRAWSURFACE7 surface, LPRECT, DWORD,
                                 LPDDBLTFX) {
  // DisplayMessage("Blt");
  return S_OK;
}

HRESULT __stdcall OglSurface_BltBatch(OglDirectDrawSurface* This, LPDDBLTBATCH, DWORD, DWORD) {
  DisplayMessage("BltBatch");
  return S_OK;
}

HRESULT __stdcall OglSurface_BltFast(OglDirectDrawSurface* This, DWORD, DWORD, LPDIRECTDRAWSURFACE7, LPRECT, DWORD) {
  DisplayMessage("BltFast");
  return S_OK;
}

HRESULT __stdcall OglSurface_DeleteAttachedSurface(OglDirectDrawSurface* This, DWORD, LPDIRECTDRAWSURFACE7) {
  DisplayMessage("DeleteAttachedSurface");
  return S_OK;
}

HRESULT __stdcall OglSurface_EnumAttachedSurfaces(OglDirectDrawSurface* This, LPVOID, LPDDENUMSURFACESCALLBACK7) {
  DisplayMessage("EnumAttachedSurfaces");
  return S_OK;
}

HRESULT __stdcall OglSurface_EnumOverlayZOrders(OglDirectDrawSurface* This, DWORD, LPVOID, LPDDENUMSURFACESCALLBACK7) {
  DisplayMessage("EnumOverlayZOrders");
  return S_OK;
}

HRESULT __stdcall OglSurface_Flip(OglDirectDrawSurface* This, LPDIRECTDRAWSURFACE7, DWORD) {
  DisplayMessage("Flip");
  return S_OK;
}

HRESULT __stdcall OglSurface_GetAttachedSurface(OglDirectDrawSurface* This, LPDDSCAPS2, LPDIRECTDRAWSURFACE7 FAR*) {
  DisplayMessage("GetAttachedSurface");
  return S_OK;
}

HRESULT __stdcall OglSurface_GetBltStatus(OglDirectDrawSurface* This, DWORD) {
  // DisplayMessage("GetBltStatus");
  return S_OK;
}

HRESULT __stdcall OglSurface_GetCaps(OglDirectDrawSurface* This, LPDDSCAPS2) {
  DisplayMessage("GetCaps");
  return S_OK;
}

HRESULT __stdcall OglSurface_GetClipper(OglDirectDrawSurface* This, LPDIRECTDRAWCLIPPER FAR*) {
  // DisplayMessage("GetClipper");
  return S_OK;
}

HRESULT __stdcall OglSurface_GetColorKey(OglDirectDrawSurface* This, DWORD, LPDDCOLORKEY) {
  DisplayMessage("GetColorKey");
  return S_OK;
}

HRESULT __stdcall OglSurface_GetDC(OglDirectDrawSurface* This, HDC FAR* hdc) {
  // DisplayMessage("GetDC");
  *hdc = (HDC)This->tex_id;
  return S_OK;
}

HRESULT __stdcall OglSurface_GetFlipStatus(OglDirectDrawSurface* This, DWORD) {
  // DisplayMessage("GetFlipStatus");
  return S_OK;
}

HRESULT __stdcall OglSurface_GetOverlayPosition(OglDirectDrawSurface* This, LPLONG, LPLONG) {
  DisplayMessage("GetOverlayPosition");
  return S_OK;
}

HRESULT __stdcall OglSurface_GetPalette(OglDirectDrawSurface* This, LPDIRECTDRAWPALETTE FAR* palette) {
  // DisplayMessage("GetPalette");
  *palette = This->palette;
  return S_OK;
}

HRESULT __stdcall OglSurface_GetPixelFormat(OglDirectDrawSurface* This, LPDDPIXELFORMAT pf) {
  // DisplayMessage("GetPixelFormat");

  if (pf) {
    *pf = This->desc.ddpfPixelFormat;
    // DisplayMessage(std::format("GetPixelFormat Mask: {}", pf->dwRBitMask));
  }

  return S_OK;
}

HRESULT __stdcall OglSurface_GetSurfaceDesc(OglDirectDrawSurface* This, LPDDSURFACEDESC2 desc) {
  // DisplayMessage("GetSurfaceDesc");
  if (desc) {
    memcpy(desc, &This->desc, This->desc.dwSize);
  }

  return S_OK;
}

HRESULT __stdcall OglSurface_Initialize(OglDirectDrawSurface* This, LPDIRECTDRAW dd, LPDDSURFACEDESC2 desc) {
  DisplayMessage("Initialize");
  This->desc = *desc;
  return S_OK;
}

HRESULT __stdcall OglSurface_IsLost(OglDirectDrawSurface* This) {
  // DisplayMessage("IsLost");
  return S_OK;
}

HRESULT __stdcall OglSurface_Lock(OglDirectDrawSurface* This, LPRECT region, LPDDSURFACEDESC2 desc, DWORD flags,
                                  HANDLE) {
  // DisplayMessage("Lock");

  while (InterlockedCompareExchange(&This->locked, 1, 0) != 0) {
    // Loop while waiting for lock
  }

  memcpy(desc, &This->desc, This->desc.dwSize);

  // desc->lpSurface = (u8*)This->buffer;
  desc->lpSurface = g_SurfaceBuffer;

#if 0
  if (region) {
    desc->lpSurface = (u8*)This->buffer + region->top * This->desc.lPitch +
      region->left * (This->desc.ddpfPixelFormat.dwRGBBitCount / 8);
  }
#endif

  return S_OK;
}

HRESULT __stdcall OglSurface_ReleaseDC(OglDirectDrawSurface* This, HDC) {
  // DisplayMessage("ReleaseDC");
  return S_OK;
}

HRESULT __stdcall OglSurface_Restore(OglDirectDrawSurface* This) {
  // DisplayMessage("Restore");
  return S_OK;
}

HRESULT __stdcall OglSurface_SetClipper(OglDirectDrawSurface* This, LPDIRECTDRAWCLIPPER) {
  // DisplayMessage("SetClipper");
  return S_OK;
}

HRESULT __stdcall OglSurface_SetColorKey(OglDirectDrawSurface* This, DWORD, LPDDCOLORKEY) {
  // DisplayMessage("SetColorKey");
  return S_OK;
}

HRESULT __stdcall OglSurface_SetOverlayPosition(OglDirectDrawSurface* This, LONG, LONG) {
  DisplayMessage("SetOverlayPosition");
  return S_OK;
}

HRESULT __stdcall OglSurface_SetPalette(OglDirectDrawSurface* This, LPDIRECTDRAWPALETTE palette) {
  // DisplayMessage("SetPalette");
  This->palette = palette;
  return S_OK;
}

HRESULT __stdcall OglSurface_Unlock(OglDirectDrawSurface* This, LPRECT) {
  // DisplayMessage("Unlock");
  InterlockedExchange(&This->locked, 0);
  return S_OK;
}

HRESULT __stdcall OglSurface_UpdateOverlay(OglDirectDrawSurface* This, LPRECT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD,
                                           LPDDOVERLAYFX) {
  DisplayMessage("UpdateOverlay");
  return S_OK;
}

HRESULT __stdcall OglSurface_UpdateOverlayDisplay(OglDirectDrawSurface* This, DWORD) {
  DisplayMessage("UpdateOverlayDisplay");
  return S_OK;
}

HRESULT __stdcall OglSurface_UpdateOverlayZOrder(OglDirectDrawSurface* This, DWORD, LPDIRECTDRAWSURFACE7) {
  DisplayMessage("UpdateOverlayZOrder");
  return S_OK;
}

HRESULT __stdcall OglSurface_GetDDInterface(OglDirectDrawSurface* This, LPVOID FAR*) {
  DisplayMessage("GetDDInterface");
  return S_OK;
}

HRESULT __stdcall OglSurface_PageLock(OglDirectDrawSurface* This, DWORD) {
  DisplayMessage("PageLock");
  return S_OK;
}

HRESULT __stdcall OglSurface_PageUnlock(OglDirectDrawSurface* This, DWORD) {
  DisplayMessage("PageUnlock");
  return S_OK;
}

HRESULT __stdcall OglSurface_SetSurfaceDesc(OglDirectDrawSurface* This, LPDDSURFACEDESC2, DWORD) {
  DisplayMessage("SetSurfaceDesc");
  return S_OK;
}

HRESULT __stdcall OglSurface_SetPrivateData(OglDirectDrawSurface* This, REFGUID, LPVOID, DWORD, DWORD) {
  DisplayMessage("SetPrivateData");
  return S_OK;
}

HRESULT __stdcall OglSurface_GetPrivateData(OglDirectDrawSurface* This, REFGUID, LPVOID, LPDWORD) {
  DisplayMessage("GetPrivateData");
  return S_OK;
}

HRESULT __stdcall OglSurface_FreePrivateData(OglDirectDrawSurface* This, REFGUID) {
  DisplayMessage("FreePrivateData");
  return S_OK;
}

HRESULT __stdcall OglSurface_GetUniquenessValue(OglDirectDrawSurface* This, LPDWORD) {
  DisplayMessage("GetUniquenessValue");
  return S_OK;
}

HRESULT __stdcall OglSurface_ChangeUniquenessValue(OglDirectDrawSurface* This) {
  DisplayMessage("ChangeUniquenessValue");
  return S_OK;
}

HRESULT __stdcall OglSurface_SetPriority(OglDirectDrawSurface* This, DWORD) {
  DisplayMessage("SetPriority");
  return S_OK;
}

HRESULT __stdcall OglSurface_GetPriority(OglDirectDrawSurface* This, LPDWORD) {
  DisplayMessage("GetPriority");
  return S_OK;
}

HRESULT __stdcall OglSurface_SetLOD(OglDirectDrawSurface* This, DWORD) {
  DisplayMessage("SetLOD");
  return S_OK;
}

HRESULT __stdcall OglSurface_GetLOD(OglDirectDrawSurface* This, LPDWORD) {
  DisplayMessage("GetLOD");
  return S_OK;
}

IDirectDrawSurface7* __stdcall OglDirectDrawCreateSurface(LPDDSURFACEDESC2 desc) {
  vtable.QueryInterface = OglSurface_QueryInterface;
  vtable.AddRef = OglSurface_AddRef;
  vtable.Release = OglSurface_Release;
  vtable.AddAttachedSurface = OglSurface_AddAttachedSurface;
  vtable.AddOverlayDirtyRect = OglSurface_AddOverlayDirtyRect;
  vtable.Blt = OglSurface_Blt;
  vtable.BltBatch = OglSurface_BltBatch;
  vtable.BltFast = OglSurface_BltFast;
  vtable.DeleteAttachedSurface = OglSurface_DeleteAttachedSurface;
  vtable.EnumAttachedSurfaces = OglSurface_EnumAttachedSurfaces;
  vtable.EnumOverlayZOrders = OglSurface_EnumOverlayZOrders;
  vtable.Flip = OglSurface_Flip;
  vtable.GetAttachedSurface = OglSurface_GetAttachedSurface;
  vtable.GetBltStatus = OglSurface_GetBltStatus;
  vtable.GetCaps = OglSurface_GetCaps;
  vtable.GetClipper = OglSurface_GetClipper;
  vtable.GetColorKey = OglSurface_GetColorKey;
  vtable.GetDC = OglSurface_GetDC;
  vtable.GetFlipStatus = OglSurface_GetFlipStatus;
  vtable.GetOverlayPosition = OglSurface_GetOverlayPosition;
  vtable.GetPalette = OglSurface_GetPalette;
  vtable.GetPixelFormat = OglSurface_GetPixelFormat;
  vtable.GetSurfaceDesc = OglSurface_GetSurfaceDesc;
  vtable.Initialize = OglSurface_Initialize;
  vtable.IsLost = OglSurface_IsLost;
  vtable.Lock = OglSurface_Lock;
  vtable.ReleaseDC = OglSurface_ReleaseDC;
  vtable.Restore = OglSurface_Restore;
  vtable.SetClipper = OglSurface_SetClipper;
  vtable.SetColorKey = OglSurface_SetColorKey;
  vtable.SetOverlayPosition = OglSurface_SetOverlayPosition;
  vtable.SetPalette = OglSurface_SetPalette;
  vtable.Unlock = OglSurface_Unlock;
  vtable.UpdateOverlay = OglSurface_UpdateOverlay;
  vtable.UpdateOverlayDisplay = OglSurface_UpdateOverlayDisplay;
  vtable.UpdateOverlayZOrder = OglSurface_UpdateOverlayZOrder;
  vtable.GetDDInterface = OglSurface_GetDDInterface;
  vtable.PageLock = OglSurface_PageLock;
  vtable.PageUnlock = OglSurface_PageUnlock;
  vtable.SetSurfaceDesc = OglSurface_SetSurfaceDesc;
  vtable.SetPrivateData = OglSurface_SetPrivateData;
  vtable.GetPrivateData = OglSurface_GetPrivateData;
  vtable.FreePrivateData = OglSurface_FreePrivateData;
  vtable.GetUniquenessValue = OglSurface_GetUniquenessValue;
  vtable.ChangeUniquenessValue = OglSurface_ChangeUniquenessValue;
  vtable.SetPriority = OglSurface_SetPriority;
  vtable.GetPriority = OglSurface_GetPriority;
  vtable.SetLOD = OglSurface_SetLOD;
  vtable.GetLOD = OglSurface_GetLOD;

  OglDirectDrawSurface* surface = (OglDirectDrawSurface*)malloc(sizeof(OglDirectDrawSurface));

  memcpy(&surface->guid, &IID_IDirectDrawSurface7, sizeof(surface->guid));
  surface->lpVtbl = &vtable;
  IDirectDrawSurface_AddRef(surface);

  surface->desc = *desc;
  surface->desc.lPitch = surface->desc.dwWidth * (surface->desc.ddpfPixelFormat.dwRGBBitCount / 8);

  surface->locked = 0;
  surface->palette = 0;

  size_t size = surface->desc.lPitch * surface->desc.dwHeight;

  surface->tex_id = OglRenderer::Get().CreateTexture();

  // DisplayMessage(std::format("Creating surface {}, {}, {}", surface->desc.dwWidth, surface->desc.dwHeight, size));

  return (IDirectDrawSurface7*)surface;
}
