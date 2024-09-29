#pragma once

#include <glad/gl.h>

namespace rogl {

struct Shader {
  bool Initialize(const char* vertex_code, const char* fragment_code);
  void Use();

  void Cleanup();

  GLuint program = -1;
};

}  // namespace rogl
