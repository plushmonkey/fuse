#pragma once

#include <fuse/Math.h>

namespace rogl {

using fuse::Vector2f;
using fuse::Vector2i;

constexpr float kPi = 3.14159265f;

inline constexpr float Radians(float degrees) {
  constexpr float kDegreeToRadians = kPi / 180.0f;
  return degrees * kDegreeToRadians;
}

inline constexpr float Degrees(float radians) {
  constexpr float kRadianToDegrees = 180.0f / kPi;
  return radians * kRadianToDegrees;
}

inline float WrapMax(float x, float max) {
  return fmodf(max + fmodf(x, max), max);
}

inline float WrapMinMax(float x, float min, float max) {
  return min + WrapMax(x - min, max - min);
}

inline float WrapToPi(float rads) {
  return WrapMinMax(rads, -3.14159f, 3.14159f);
}

inline constexpr float min(float left, float right) {
  return right < left ? right : left;
}

inline constexpr float max(float left, float right) {
  return left < right ? right : left;
}

struct Rectangle2f {
  Vector2f min;
  Vector2f max;

  Rectangle2f() {}
  Rectangle2f(Vector2f min, Vector2f max) : min(min), max(max) {}

  inline Vector2f GetExtent() const { return max - min; }

  inline Rectangle2f Align() {
    int min_x = (int)min.x;
    int min_y = (int)min.y;
    int max_x = (int)max.x;
    int max_y = (int)max.y;

    return Rectangle2f(Vector2f((float)min_x, (float)min_y), Vector2f((float)max_x, (float)max_y));
  }

  inline Rectangle2f Translate(Vector2f v) const { return Rectangle2f(min + v, max + v); }
  inline Rectangle2f Scale(float scale) const {
    Vector2f center = (max + min) * 0.5f;
    Vector2f half_extents = (max - min) * (scale * 0.5f);
    return Rectangle2f(center - half_extents, center + half_extents);
  }
  inline Rectangle2f Grow(Vector2f amount) const { return Rectangle2f(min - amount, max + amount); }
  inline Rectangle2f Grow(float amount) const {
    return Rectangle2f(min - Vector2f(amount, amount), max + Vector2f(amount, amount));
  }

  inline Vector2f GetCenter() const { return (min + max) * 0.5f; }

  inline bool Contains(Vector2f point) const {
    return (point.x >= min.x && point.x < max.x) && (point.y >= min.y && point.y < max.y);
  }

  inline bool ContainsExclusive(Vector2f point) const {
    return (point.x > min.x && point.x < max.x) && (point.y > min.y && point.y < max.y);
  }

  inline bool ContainsInclusive(Vector2f point) const {
    return (point.x >= min.x && point.x <= max.x) && (point.y >= min.y && point.y <= max.y);
  }

  inline static Rectangle2f FromPositionRadius(const Vector2f& position, float radius) {
    return Rectangle2f(position - Vector2f(radius, radius), position + Vector2f(radius, radius));
  }
};

struct Vector3f {
  union {
    struct {
      float x;
      float y;
      float z;
    };
    float values[3];
  };

  constexpr Vector3f() : x(0), y(0), z(0) {}
  constexpr Vector3f(float x, float y, float z) : x(x), y(y), z(z) {}
  constexpr Vector3f(const Vector2f& v2, float z) : x(v2.x), y(v2.y), z(z) {}
  constexpr Vector3f(const Vector3f& other) : x(other.x), y(other.y), z(other.z) {}

  Vector3f& operator=(const Vector3f& other) {
    x = other.x;
    y = other.y;
    z = other.z;

    return *this;
  }

  inline bool operator==(const Vector3f& other) { return x == other.x && y == other.y && z == other.z; }

  inline bool operator!=(const Vector3f& other) { return !(x == other.x && y == other.y && z == other.z); }

  inline float& operator[](size_t index) { return values[index]; }

  inline float operator[](size_t index) const { return values[index]; }

  inline Vector3f& operator+=(float value) {
    x += value;
    y += value;
    z += value;
    return *this;
  }

  inline Vector3f& operator-=(float value) {
    x -= value;
    y -= value;
    z -= value;
    return *this;
  }

  inline Vector3f& operator+=(const Vector3f& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
  }

  inline Vector3f& operator-=(const Vector3f& other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
  }

  inline Vector3f& operator*=(float value) {
    x *= value;
    y *= value;
    z *= value;
    return *this;
  }

  inline Vector3f& operator/=(float value) {
    x /= value;
    y /= value;
    z /= value;
    return *this;
  }

  inline Vector3f operator+(const Vector3f& other) const { return Vector3f(x + other.x, y + other.y, z + other.z); }

  inline Vector3f operator-(const Vector3f& other) const { return Vector3f(x - other.x, y - other.y, z - other.z); }

  inline Vector3f operator-() const { return Vector3f(-x, -y, -z); }

  inline Vector3f operator*(float value) const { return Vector3f(x * value, y * value, z * value); }

  inline Vector3f operator/(float value) const { return Vector3f(x / value, y / value, z / value); }

  inline float Length() const { return sqrtf(x * x + y * y + z * z); }

  inline float LengthSq() const { return x * x + y * y + z * z; }

  inline float Distance(const Vector3f& other) const {
    float dx = x - other.x;
    float dy = y - other.y;
    float dz = z - other.z;

    return sqrtf(dx * dx + dy * dy + dz * dz);
  }

  inline float DistanceSq(const Vector3f& other) const {
    float dx = x - other.x;
    float dy = y - other.y;
    float dz = y - other.z;

    return dx * dx + dy * dy + dz * dz;
  }

  inline float Dot(const Vector3f& other) const { return x * other.x + y * other.y + z * other.z; }

  inline Vector3f Cross(const Vector3f& other) const {
    return Vector3f(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
  }

  inline Vector3f& Normalize() {
    float length = Length();
    if (length > 0) {
      x /= length;
      y /= length;
      z /= length;
    }
    return *this;
  }

  inline Vector3f& Truncate(float length) {
    if (LengthSq() > length * length) {
      Normalize();
      *this *= length;
    }
    return *this;
  }
};

inline Vector3f operator*(float value, const Vector3f& v) {
  return Vector3f(v.x * value, v.y * value, v.z * value);
}

inline float Dot(const Vector3f& v1, const Vector3f& v2) {
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

inline Vector3f Cross(const Vector3f& v1, const Vector3f& v2) {
  return Vector3f(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}

inline Vector3f Normalize(const Vector3f& v) {
  float length = v.Length();

  if (length > 0) {
    return Vector3f(v.x / length, v.y / length, v.z / length);
  }

  return v;
}

struct Vector4f {
  union {
    struct {
      float x;
      float y;
      float z;
      float w;
    };
    float values[4];
  };

  Vector4f() : x(0), y(0), z(0), w(0) {}
  Vector4f(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
  Vector4f(const Vector2f& v2, float z, float w) : x(v2.x), y(v2.y), z(z), w(w) {}
  Vector4f(const Vector3f& other, float w) : x(other.x), y(other.y), z(other.z), w(w) {}

  Vector4f& operator=(const Vector4f& other) {
    x = other.x;
    y = other.y;
    z = other.z;
    w = other.w;

    return *this;
  }

  inline bool operator==(const Vector4f& other) { return x == other.x && y == other.y && z == other.z && w == other.w; }

  inline bool operator!=(const Vector4f& other) {
    return !(x == other.x && y == other.y && z == other.z && w == other.w);
  }

  inline float& operator[](size_t index) { return values[index]; }

  inline float operator[](size_t index) const { return values[index]; }

  inline Vector4f operator*(float value) { return Vector4f(x * value, y * value, z * value, w * value); }

  inline Vector4f operator+(const Vector4f& other) {
    return Vector4f(x + other.x, y + other.y, z + other.z, w + other.w);
  }
};

// Column major matrix4x4
struct mat4 {
  float data[4][4];

  mat4() : data{} {}
  mat4(float v) : data{} {
    for (size_t i = 0; i < 4; ++i) {
      data[i][i] = v;
    }
  }
  mat4(float* data) { memcpy(this->data, data, sizeof(mat4)); }

  float* operator[](size_t index) { return data[index]; }

  const float* operator[](size_t index) const { return data[index]; }

  Vector4f Multiply(const Vector3f& v, float w) {
    Vector4f result;

    for (size_t row = 0; row < 4; ++row) {
      result[row] = v.x * data[0][row] + v.y * data[1][row] + v.z * data[2][row] + w * data[3][row];
    }

    return result;
  }

  // Constructs the matrix from column vectors.
  // The columns are for operational order not for internal order (column-major
  // stored)
  static mat4 FromColumns(const Vector4f& x, const Vector4f& y, const Vector4f& z, const Vector4f& w) {
    mat4 result;

    for (size_t i = 0; i < 4; ++i) {
      result[0][i] = x[i];
      result[1][i] = y[i];
      result[2][i] = z[i];
      result[3][i] = w[i];
    }

    return result;
  }

  // Constructs the matrix from row vectors.
  // The rows are for operational order not for internal order (column-major
  // stored)
  static mat4 FromRows(const Vector4f& x, const Vector4f& y, const Vector4f& z, const Vector4f& w) {
    mat4 result;

    for (size_t i = 0; i < 4; ++i) {
      result[i][0] = x[i];
      result[i][1] = y[i];
      result[i][2] = z[i];
      result[i][3] = w[i];
    }

    return result;
  }

  static mat4 Identity() {
    mat4 result = {};

    result.data[0][0] = 1;
    result.data[1][1] = 1;
    result.data[2][2] = 1;
    result.data[3][3] = 1;

    return result;
  }
};

inline mat4 operator*(const mat4& M1, const mat4& M2) {
  mat4 result = {};

  for (size_t col = 0; col < 4; ++col) {
    for (size_t row = 0; row < 4; ++row) {
      for (size_t i = 0; i < 4; ++i) {
        result.data[col][row] += M1.data[i][row] * M2.data[col][i];
      }
    }
  }

  return result;
}

inline mat4 LookAt(const Vector3f& eye, const Vector3f& to, Vector3f world_up = Vector3f(0, 1, 0)) {
  // Compute camera axes
  Vector3f forward = Normalize(to - eye);
  Vector3f side = Normalize(forward.Cross(world_up));
  Vector3f up = Normalize(side.Cross(forward));

  // Insert camera axes in column major order and transform eye into the camera
  // space for translation
  float values[] = {side.x, up.x, -forward.x, 0, side.y,          up.y,          -forward.y,        0,
                    side.z, up.z, -forward.z, 0, -Dot(side, eye), -Dot(up, eye), Dot(forward, eye), 1};

  return mat4(values);
}

inline mat4 Translate(const mat4& M, const Vector3f& translation) {
  float values[] = {
      1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, translation.x, translation.y, translation.z, 1,
  };

  mat4 translate_op((float*)values);
  return translate_op * M;
}

inline mat4 Scale(const mat4& M, const Vector3f& scale) {
  float values[] = {
      scale.x, 0, 0, 0, 0, scale.y, 0, 0, 0, 0, scale.z, 0, 0, 0, 0, 1,
  };

  mat4 scale_op((float*)values);
  return scale_op * M;
}

// fov: field of view for y-axis
// aspect_ratio: width / height
// near: near plane in camera space
// far: far plane in camera space
inline mat4 Perspective(float fov, float aspect_ratio, float near_plane, float far_plane) {
  float half_tan = tanf(fov / 2.0f);

  float values[] = {
      1.0f / (aspect_ratio * half_tan),
      0,
      0,
      0,
      0,
      -1.0f / half_tan,
      0,
      0,
      0,
      0,
      -(far_plane + near_plane) / (far_plane - near_plane),
      -1.0f,
      0,
      0,
      -(2.0f * far_plane * near_plane) / (far_plane - near_plane),
      0,
  };

  return mat4(values);
}

inline mat4 Orthographic(float left, float right, float bottom, float top, float near_plane, float far_plane) {
  float values[] = {
      2.0f / (right - left),
      0,
      0,
      0,
      0,
      2 / (top - bottom),
      0,
      0,
      0,
      0,
      -2.0f / (far_plane - near_plane),
      0,
      -(right + left) / (right - left),
      -(top + bottom) / (top - bottom),
      -(far_plane + near_plane) / (far_plane - near_plane),
      1,
  };

  return mat4(values);
}

inline mat4 Rotate(const mat4& M, float angle, const Vector3f& rotate_axis) {
  float c = cosf(angle);
  float s = sinf(angle);

  Vector3f axis = Normalize(rotate_axis);
  Vector3f t = (1.0f - c) * axis;

  float rotator[3][3];

  rotator[0][0] = c + t[0] * axis[0];
  rotator[0][1] = t[0] * axis[1] + s * axis[2];
  rotator[0][2] = t[0] * axis[2] - s * axis[1];

  rotator[1][0] = t[1] * axis[0] - s * axis[2];
  rotator[1][1] = c + t[1] * axis[1];
  rotator[1][2] = t[1] * axis[2] + s * axis[0];

  rotator[2][0] = t[2] * axis[0] + s * axis[1];
  rotator[2][1] = t[2] * axis[1] - s * axis[0];
  rotator[2][2] = c + t[2] * axis[2];

  Vector4f M0(M[0][0], M[0][1], M[0][2], M[0][3]);
  Vector4f M1(M[1][0], M[1][1], M[1][2], M[1][3]);
  Vector4f M2(M[2][0], M[2][1], M[2][2], M[2][3]);

  Vector4f R0 = M0 * rotator[0][0] + M1 * rotator[0][1] + M2 * rotator[0][2];
  Vector4f R1 = M0 * rotator[1][0] + M1 * rotator[1][1] + M2 * rotator[1][2];
  Vector4f R2 = M0 * rotator[2][0] + M1 * rotator[2][1] + M2 * rotator[2][2];

  float values[] = {R0[0], R0[1], R0[2], R0[3], R1[0],   R1[1],   R1[2],   R2[3],
                    R2[0], R2[1], R2[2], R1[3], M[3][0], M[3][1], M[3][2], M[3][3]};

  return mat4((float*)values);
}

inline Vector4f operator*(const mat4& M, const Vector4f& v) {
  Vector4f result;

  for (size_t row = 0; row < 4; ++row) {
    result[row] = v.x * M.data[0][row] + v.y * M.data[1][row] + v.z * M.data[2][row] + v.w * M.data[3][row];
  }

  return result;
}

}  // namespace rogl
