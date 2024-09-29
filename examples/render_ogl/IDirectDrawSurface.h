#pragma once

#ifndef CINTERFACE
#define CINTERFACE
#define FUSE_MANUAL_CINTERFACE
#endif

#include <ddraw.h>

#ifdef INTERFACE
#undef INTERFACE
#endif
#define INTERFACE OglDirectDrawSurface

struct OglDirectDrawSurface;

struct OglDirectDrawSurfaceVtable {
  /*** IUnknown methods ***/
  STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;  // 0
  STDMETHOD_(ULONG, AddRef)(THIS) PURE;                                   // 1
  STDMETHOD_(ULONG, Release)(THIS) PURE;                                  // 2
  /*** IDirectDrawSurface methods ***/
  STDMETHOD(AddAttachedSurface)(THIS_ LPDIRECTDRAWSURFACE7) PURE;                                   // 3
  STDMETHOD(AddOverlayDirtyRect)(THIS_ LPRECT) PURE;                                                // 4
  STDMETHOD(Blt)(THIS_ LPRECT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD, LPDDBLTFX) PURE;                // 5
  STDMETHOD(BltBatch)(THIS_ LPDDBLTBATCH, DWORD, DWORD) PURE;                                       // 6
  STDMETHOD(BltFast)(THIS_ DWORD, DWORD, LPDIRECTDRAWSURFACE7, LPRECT, DWORD) PURE;                 // 7
  STDMETHOD(DeleteAttachedSurface)(THIS_ DWORD, LPDIRECTDRAWSURFACE7) PURE;                         // 8
  STDMETHOD(EnumAttachedSurfaces)(THIS_ LPVOID, LPDDENUMSURFACESCALLBACK7) PURE;                    // 9
  STDMETHOD(EnumOverlayZOrders)(THIS_ DWORD, LPVOID, LPDDENUMSURFACESCALLBACK7) PURE;               // A
  STDMETHOD(Flip)(THIS_ LPDIRECTDRAWSURFACE7, DWORD) PURE;                                          // B
  STDMETHOD(GetAttachedSurface)(THIS_ LPDDSCAPS2, LPDIRECTDRAWSURFACE7 FAR*) PURE;                  // C
  STDMETHOD(GetBltStatus)(THIS_ DWORD) PURE;                                                        // D
  STDMETHOD(GetCaps)(THIS_ LPDDSCAPS2) PURE;                                                        // E
  STDMETHOD(GetClipper)(THIS_ LPDIRECTDRAWCLIPPER FAR*) PURE;                                       // F
  STDMETHOD(GetColorKey)(THIS_ DWORD, LPDDCOLORKEY) PURE;                                           // 10
  STDMETHOD(GetDC)(THIS_ HDC FAR*) PURE;                                                            // 11
  STDMETHOD(GetFlipStatus)(THIS_ DWORD) PURE;                                                       // 12
  STDMETHOD(GetOverlayPosition)(THIS_ LPLONG, LPLONG) PURE;                                         // 13
  STDMETHOD(GetPalette)(THIS_ LPDIRECTDRAWPALETTE FAR*) PURE;                                       // 14
  STDMETHOD(GetPixelFormat)(THIS_ LPDDPIXELFORMAT) PURE;                                            // 15
  STDMETHOD(GetSurfaceDesc)(THIS_ LPDDSURFACEDESC2) PURE;                                           // 16
  STDMETHOD(Initialize)(THIS_ LPDIRECTDRAW, LPDDSURFACEDESC2) PURE;                                 // 17
  STDMETHOD(IsLost)(THIS) PURE;                                                                     // 18
  STDMETHOD(Lock)(THIS_ LPRECT, LPDDSURFACEDESC2, DWORD, HANDLE) PURE;                              // 19
  STDMETHOD(ReleaseDC)(THIS_ HDC) PURE;                                                             // 1A
  STDMETHOD(Restore)(THIS) PURE;                                                                    // 1B
  STDMETHOD(SetClipper)(THIS_ LPDIRECTDRAWCLIPPER) PURE;                                            // 1C
  STDMETHOD(SetColorKey)(THIS_ DWORD, LPDDCOLORKEY) PURE;                                           // 1D
  STDMETHOD(SetOverlayPosition)(THIS_ LONG, LONG) PURE;                                             // 1E
  STDMETHOD(SetPalette)(THIS_ LPDIRECTDRAWPALETTE) PURE;                                            // 1F
  STDMETHOD(Unlock)(THIS_ LPRECT) PURE;                                                             // 20
  STDMETHOD(UpdateOverlay)(THIS_ LPRECT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD, LPDDOVERLAYFX) PURE;  // 21
  STDMETHOD(UpdateOverlayDisplay)(THIS_ DWORD) PURE;                                                // 22
  STDMETHOD(UpdateOverlayZOrder)(THIS_ DWORD, LPDIRECTDRAWSURFACE7) PURE;                           // 23
  /*** Added in the v2 interface ***/
  STDMETHOD(GetDDInterface)(THIS_ LPVOID FAR*) PURE;  // 24
  STDMETHOD(PageLock)(THIS_ DWORD) PURE;              // 25
  STDMETHOD(PageUnlock)(THIS_ DWORD) PURE;            // 26
  /*** Added in the v3 interface ***/
  STDMETHOD(SetSurfaceDesc)(THIS_ LPDDSURFACEDESC2, DWORD) PURE;  // 27
  /*** Added in the v4 interface ***/
  STDMETHOD(SetPrivateData)(THIS_ REFGUID, LPVOID, DWORD, DWORD) PURE;  // 28
  STDMETHOD(GetPrivateData)(THIS_ REFGUID, LPVOID, LPDWORD) PURE;       // 29
  STDMETHOD(FreePrivateData)(THIS_ REFGUID) PURE;                       // 2A
  STDMETHOD(GetUniquenessValue)(THIS_ LPDWORD) PURE;                    // 2B
  STDMETHOD(ChangeUniquenessValue)(THIS) PURE;                          // 2C
  /*** Moved Texture7 methods here ***/
  STDMETHOD(SetPriority)(THIS_ DWORD) PURE;    // 2D
  STDMETHOD(GetPriority)(THIS_ LPDWORD) PURE;  // 2E
  STDMETHOD(SetLOD)(THIS_ DWORD) PURE;         // 2F
  STDMETHOD(GetLOD)(THIS_ LPDWORD) PURE;       // 30
};

struct OglDirectDrawSurface {
  OglDirectDrawSurfaceVtable* lpVtbl;

  ULONG ref;
  GUID guid;

  DDSURFACEDESC2 desc;

  void* buffer;

  IDirectDrawPalette* palette;
  unsigned int locked;
};

IDirectDrawSurface7* __stdcall OglDirectDrawCreateSurface(LPDDSURFACEDESC2 desc);

#ifdef FUSE_MANUAL_CINTERFACE
#undef CINTERFACE
#undef FUSE_MANUAL_CINTERFACE
#endif

#ifdef INTERFACE
#undef INTERFACE
#endif
