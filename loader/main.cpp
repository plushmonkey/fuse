#include "DirectSound.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <fstream>
#include <iostream>
#include <string>

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

      fuse::g_DirectSound = fuse::DirectSound::Load(dsound);

      InitializeLoader();
    } break;
    case DLL_PROCESS_DETACH: {
      FreeLibrary(dsound);
    } break;
  }

  return TRUE;
}
