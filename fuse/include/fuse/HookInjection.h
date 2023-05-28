#pragma once

namespace fuse {

class HookInjection {
 public:
  virtual void OnUpdate() = 0;

  // Return true if the key should be pressed
  virtual bool OnGetAsyncKeyState(int vKey) = 0;
};

}  // namespace fuse
