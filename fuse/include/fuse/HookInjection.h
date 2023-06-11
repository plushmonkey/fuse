#pragma once

#include <fuse/Math.h>
#include <fuse/Platform.h>

namespace fuse {

struct KeyState {
  bool forced;
  bool pressed;
};

class HookInjection {
 public:
  // This runs before GetMessage is called. If it returns true then it will bypass GetMessage so a custom message can be
  // returned.
  virtual bool OnGetMessage(LPMSG lpMsg, HWND hWnd) { return false; };

  // This runs before PeekMessage is called. If it returns true then it will bypass PeekMessage so a custom message can
  // be returned.
  virtual bool OnPeekMessage(LPMSG lpMsg, HWND hWnd) { return false; };

  virtual void OnUpdate(){};

  // This is called after OnUpdate and PeekMessage complete.
  virtual bool OnPostUpdate(BOOL hasMsg, LPMSG lpMsg, HWND hWnd) { return false; };

  virtual bool OnMenuUpdate(BOOL hasMsg, LPMSG lpMsg, HWND hWnd) { return false; };

  virtual void OnMouseMove(const Vector2i& position, MouseButtons buttons) {}
  virtual void OnMouseDown(const Vector2i& position, MouseButton button) {}
  virtual void OnMouseUp(const Vector2i& position, MouseButton button) {}

  // Return true if the key should be pressed
  virtual KeyState OnGetAsyncKeyState(int vKey) { return {}; }
};

}  // namespace fuse
