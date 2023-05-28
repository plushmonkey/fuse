#pragma once

namespace fuse {

struct KeyState {
  bool forced;
  bool pressed;
};

class HookInjection {
 public:
  virtual void OnUpdate() = 0;

  // Return true if the key should be pressed
  virtual KeyState OnGetAsyncKeyState(int vKey) = 0;
};

}  // namespace fuse
