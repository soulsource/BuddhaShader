#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>

namespace Helpers
{
	GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path);
	GLuint LoadComputeShader(const char * compute_file_path);

    void WriteOutputPNG(const std::vector<uint32_t>& data, unsigned int width, unsigned int height);
}
