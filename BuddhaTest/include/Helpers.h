#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Helpers
{
	GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path);
	GLuint LoadComputeShader(const char * compute_file_path);
}