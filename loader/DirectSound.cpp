#include "DirectSound.h"

#include <dsound.h>
#pragma comment(lib, "dsound.lib")

namespace fuse {

DirectSound DirectSound::Load(HMODULE dsound) {
  DirectSound result = {};

  result.Create = (DirectSound::CreateProc)GetProcAddress(dsound, "DirectSoundCreate");
  result.EnumerateA = (DirectSound::EnumerateAProc)GetProcAddress(dsound, "DirectSoundEnumerateA");
  result.EnumerateW = (DirectSound::EnumerateWProc)GetProcAddress(dsound, "DirectSoundEnumerateW");
  result.DllCanUnloadNow = (DirectSound::DllCanUnloadNowProc)GetProcAddress(dsound, "DllCanUnloadNow");
  result.DllGetClassObject = (DirectSound::DllGetClassObjectProc)GetProcAddress(dsound, "DllGetClassObject");
  result.CaptureCreate = (DirectSound::CaptureCreateProc)GetProcAddress(dsound, "DirectSoundCaptureCreate");
  result.CaptureEnumerateA = (DirectSound::CaptureEnumerateAProc)GetProcAddress(dsound, "DirectSoundCaptureEnumerateA");
  result.CaptureEnumerateW = (DirectSound::CaptureEnumerateWProc)GetProcAddress(dsound, "DirectSoundCaptureEnumerateW");
  result.GetDeviceID = (DirectSound::GetDeviceIDProc)GetProcAddress(dsound, "GetDeviceID");
  result.FullDuplexCreate = (DirectSound::FullDuplexCreateProc)GetProcAddress(dsound, "DirectSoundFullDuplexCreate");
  result.Create8 = (DirectSound::Create8Proc)GetProcAddress(dsound, "DirectSoundCreate8");
  result.CaptureCreate8 = (DirectSound::CaptureCreate8Proc)GetProcAddress(dsound, "DirectSoundCaptureCreate8");

  return result;
}

}  // namespace fuse
