#include "DirectSound.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <fstream>
#include <iostream>
#include <string>

fuse::DirectSound g_DirectSound;
std::ofstream debug_log;

const char* kLoadList[] = {"marvin.dll"};

static std::string GetLocalPath() {
  char path[MAX_PATH];

  GetCurrentDirectory(MAX_PATH, path);

  return std::string(path);
}

static std::string GetSystemLibrary(const char* library) {
  std::string result;

  char system_path[MAX_PATH];

  GetSystemDirectory(system_path, MAX_PATH);
  result = std::string(system_path) + "\\" + std::string(library);

  return result;
}

extern "C" {

HRESULT __stdcall FuseDirectSoundCreate(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN pUnkOuter) {
  return g_DirectSound.Create(lpGuid, ppDS, pUnkOuter);
}

HRESULT __stdcall FuseDirectSoundEnumerateA(LPDSENUMCALLBACKA lpDSEnumCallback, LPVOID lpContext) {
  return g_DirectSound.EnumerateA(lpDSEnumCallback, lpContext);
}

HRESULT __stdcall FuseDirectSoundEnumerateW(LPDSENUMCALLBACKW lpDSEnumCallback, LPVOID lpContext) {
  return g_DirectSound.EnumerateW(lpDSEnumCallback, lpContext);
}

HRESULT __stdcall FuseDllCanUnloadNow() {
  return g_DirectSound.DllCanUnloadNow();
}

HRESULT __stdcall FuseDllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID FAR* ppv) {
  return g_DirectSound.DllGetClassObject(rclsid, riid, ppv);
}

HRESULT __stdcall FuseGetDeviceID(LPCGUID pGuidSrc, LPGUID pGuidDest) {
  return g_DirectSound.GetDeviceID(pGuidSrc, pGuidDest);
}

HRESULT __stdcall FuseDirectSoundCaptureCreate(LPCGUID lpcGUID, LPDIRECTSOUNDCAPTURE8* lplpDSC, LPUNKNOWN pUnkOuter) {
  return g_DirectSound.CaptureCreate(lpcGUID, lplpDSC, pUnkOuter);
}

HRESULT __stdcall FuseDirectSoundCaptureEnumerateA(LPDSENUMCALLBACKA lpDSEnumCallback, LPVOID lpContext) {
  return g_DirectSound.CaptureEnumerateA(lpDSEnumCallback, lpContext);
}

HRESULT __stdcall FuseDirectSoundCaptureEnumerateW(LPDSENUMCALLBACKW lpDSEnumCallback, LPVOID lpContext) {
  return g_DirectSound.CaptureEnumerateW(lpDSEnumCallback, lpContext);
}

HRESULT __stdcall FuseDirectSoundFullDuplexCreate(LPCGUID pcGuidCaptureDevice, LPCGUID pcGuidRenderDevice,
                                                  LPCDSCBUFFERDESC pcDSCBufferDesc, LPCDSBUFFERDESC pcDSBufferDesc,
                                                  HWND hWnd, DWORD dwLevel, LPDIRECTSOUNDFULLDUPLEX* ppDSFD,
                                                  LPDIRECTSOUNDCAPTUREBUFFER8* ppDSCBuffer8,
                                                  LPDIRECTSOUNDBUFFER8* ppDSBuffer8, LPUNKNOWN pUnkOuter) {
  return g_DirectSound.FullDuplexCreate(pcGuidCaptureDevice, pcGuidRenderDevice, pcDSCBufferDesc, pcDSBufferDesc, hWnd,
                                        dwLevel, ppDSFD, ppDSCBuffer8, ppDSBuffer8, pUnkOuter);
}

HRESULT __stdcall FuseDirectSoundCreate8(LPCGUID lpcGuidDevice, LPDIRECTSOUND8* ppDS8, LPUNKNOWN pUnkOuter) {
  return g_DirectSound.Create8(lpcGuidDevice, ppDS8, pUnkOuter);
}

HRESULT __stdcall FuseDirectSoundCaptureCreate8(LPCGUID lpcGUID, LPDIRECTSOUNDCAPTURE8* lplpDSC, LPUNKNOWN pUnkOuter) {
  return g_DirectSound.CaptureCreate8(lpcGUID, lplpDSC, pUnkOuter);
}

}  // extern "C"

void InitializeLoader() {
  debug_log.open("fuse_loader.log", std::ios::out);

  std::string base_path = GetLocalPath();

  // TODO: Grab these from conf file
  size_t load_count = sizeof(kLoadList) / sizeof(*kLoadList);
  debug_log << "Attempting to load " << load_count << (load_count == 1 ? " library" : " libraries") << std::endl;

  for (size_t i = 0; i < load_count; ++i) {
    std::string full_path = base_path + "\\" + std::string(kLoadList[i]);

    HMODULE loaded_module = LoadLibrary(full_path.c_str());
    debug_log << full_path << ": " << (loaded_module ? "Success" : "Failure") << std::endl;
  }
}

static HMODULE dsound;

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID reserved) {
  switch (dwReason) {
    case DLL_PROCESS_ATTACH: {
      std::string dsound_path = GetSystemLibrary("dsound.dll");
      dsound = LoadLibrary(dsound_path.c_str());

      if (!dsound) {
        MessageBox(NULL, "Failed to load system dsound.dll", "fuse", MB_ICONERROR | MB_OK);
        return FALSE;
      }

      g_DirectSound = fuse::DirectSound::Load(dsound);

      InitializeLoader();
    } break;
    case DLL_PROCESS_DETACH: {
      FreeLibrary(dsound);
    } break;
  }

  return TRUE;
}
