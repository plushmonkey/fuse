#ifndef CINTERFACE
#define CINTERFACE
#endif

#include "IDirectDrawPalette.h"

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

#define INTERFACE OglDirectDrawPalette

using namespace fuse;

static OglDirectDrawPaletteVtable vtable = {};

static void DisplayMessage(std::string_view msg) {
  MessageBox(NULL, msg.data(), "ogl_DirectDrawPalette", MB_OK);
}

HRESULT __stdcall OglPalette_QueryInterface(OglDirectDrawPalette* This, REFIID riid, LPVOID FAR* ppvObj) {
  if (!*ppvObj) return E_INVALIDARG;

  if (!IsEqualGUID(riid, IID_IDirectDrawPalette)) {
    return E_NOINTERFACE;
  }

  OglDirectDrawPalette* palette = (OglDirectDrawPalette*)malloc(sizeof(OglDirectDrawPalette));

  memcpy(&palette->guid, &riid, sizeof(palette->guid));
  palette->lpVtbl = &vtable;
  palette->ref = 1;

  *ppvObj = palette;

  return S_OK;
}

ULONG __stdcall OglPalette_AddRef(OglDirectDrawPalette* This) {
  ULONG ret = ++This->ref;

  return ret;
}

ULONG __stdcall OglPalette_Release(OglDirectDrawPalette* This) {
  ULONG ret = --This->ref;

  if (ret == 0) {
    free(This);
  }

  return ret;
}

HRESULT __stdcall OglPalette_GetCaps(OglDirectDrawPalette* This, LPDWORD) {
  DisplayMessage("GetCaps");
  return S_OK;
}

HRESULT __stdcall OglPalette_GetEntries(OglDirectDrawPalette* This, DWORD, DWORD, DWORD, LPPALETTEENTRY) {
  DisplayMessage("GetEntries");
  return S_OK;
}

HRESULT __stdcall OglPalette_Initialize(OglDirectDrawPalette* This, LPDIRECTDRAW, DWORD, LPPALETTEENTRY) {
  DisplayMessage("Initialize");
  return S_OK;
}

HRESULT __stdcall OglPalette_SetEntries(OglDirectDrawPalette* This, DWORD, DWORD, DWORD, LPPALETTEENTRY) {
  DisplayMessage("SetEntries");
  return S_OK;
}

IDirectDrawPalette* __stdcall OglDirectDrawCreatePalette(LPPALETTEENTRY entry) {
  vtable.QueryInterface = OglPalette_QueryInterface;
  vtable.AddRef = OglPalette_AddRef;
  vtable.Release = OglPalette_Release;
  vtable.GetCaps = OglPalette_GetCaps;
  vtable.GetEntries = OglPalette_GetEntries;
  vtable.Initialize = OglPalette_Initialize;
  vtable.SetEntries = OglPalette_SetEntries;

  OglDirectDrawPalette* palette = (OglDirectDrawPalette*)malloc(sizeof(OglDirectDrawPalette));

  memcpy(&palette->guid, &IID_IDirectDrawPalette, sizeof(palette->guid));
  palette->lpVtbl = &vtable;
  IDirectDrawSurface_AddRef(palette);

  return (IDirectDrawPalette*)palette;
}
