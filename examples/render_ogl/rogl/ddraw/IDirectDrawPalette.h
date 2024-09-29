#pragma once

#ifndef CINTERFACE
#define CINTERFACE
#define FUSE_MANUAL_CINTERFACE
#endif

#include <ddraw.h>

#ifdef INTERFACE
#undef INTERFACE
#endif
#define INTERFACE OglDirectDrawPalette

struct OglDirectDrawPalette;

struct OglDirectDrawPaletteVtable {
  /*** IUnknown methods ***/
  STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
  STDMETHOD_(ULONG, AddRef)(THIS) PURE;
  STDMETHOD_(ULONG, Release)(THIS) PURE;
  STDMETHOD(GetCaps)(THIS_ LPDWORD) PURE;
  STDMETHOD(GetEntries)(THIS_ DWORD, DWORD, DWORD, LPPALETTEENTRY) PURE;
  STDMETHOD(Initialize)(THIS_ LPDIRECTDRAW, DWORD, LPPALETTEENTRY) PURE;
  STDMETHOD(SetEntries)(THIS_ DWORD, DWORD, DWORD, LPPALETTEENTRY) PURE;
};

struct OglDirectDrawPalette {
  OglDirectDrawPaletteVtable* lpVtbl;

  ULONG ref;
  GUID guid;

  PALETTEENTRY palette;
};

IDirectDrawPalette* __stdcall OglDirectDrawCreatePalette(LPPALETTEENTRY);

#ifdef FUSE_MANUAL_CINTERFACE
#undef CINTERFACE
#undef FUSE_MANUAL_CINTERFACE
#endif

#ifdef INTERFACE
#undef INTERFACE
#endif
