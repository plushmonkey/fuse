#pragma once

namespace fuse {

struct KeyState {
  bool forced;
  bool pressed;
};

class HookInjection {
 public:
  virtual void OnUpdate(){};
  virtual void OnMenuUpdate(){};

  // Return true if the key should be pressed
  virtual KeyState OnGetAsyncKeyState(int vKey) { return {}; }
};

}  // namespace fuse
