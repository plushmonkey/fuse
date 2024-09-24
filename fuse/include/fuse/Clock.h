#pragma once

#include <fuse/Platform.h>
#include <fuse/Types.h>

namespace fuse {

#define TICK_DIFF(a, b) ((signed int)(((a) << 1) - ((b) << 1)) >> 1)
#define TICK_GT(a, b) (TICK_DIFF(a, b) > 0)
#define TICK_GTE(a, b) (TICK_DIFF(a, b) >= 0)
#define MAKE_TICK(a) ((a) & 0x7FFFFFFF)

struct Tick {
  u32 value = 0;

  Tick() : value(0) {}
  Tick(u32 value) : value(value) {}
  Tick(const Tick& other) : value(other.value) {}

  inline bool operator<(const Tick& other) const { return !TICK_GTE(value, other.value); }
  inline bool operator<=(const Tick& other) const { return !TICK_GT(value, other.value); }

  inline bool operator>(const Tick& other) const { return TICK_GT(value, other.value); }
  inline bool operator>=(const Tick& other) const { return TICK_GTE(value, other.value); }

  inline Tick operator+(const Tick& other) const { return Tick(MAKE_TICK(value + other.value)); }
  inline Tick operator+(u32 other) const { return Tick(MAKE_TICK(value + other)); }

  inline Tick operator-(const Tick& other) const { return Tick(MAKE_TICK(value - other.value)); }
  inline Tick operator-(u32 other) const { return Tick(MAKE_TICK(value - other)); }
};

inline Tick operator+(u32 value, Tick tick) {
  return tick + value;
}

inline Tick operator-(u32 value, Tick tick) {
  return tick - value;
}

inline Tick GetCurrentTick() {
  return Tick(MAKE_TICK(GetTickCount() / 10));
}

}  // namespace fuse
