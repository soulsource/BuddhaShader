#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <stdio.h> //includes FILE typedef

namespace Helpers
{
	GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path);
    GLuint LoadComputeShader(const char * compute_file_path, unsigned int localSizeX, unsigned int localSizeY, unsigned int localSizeZ);

    void WriteOutputPNG(const std::vector<uint32_t>& data, unsigned int width, unsigned int bufferHeight, double gamma);

    /** Wraps around a C file descriptor. Libpng could be taught to use C++ streams, but I'm too lazy and rather wrap this ugly thing up, so it gets cleaned... */
    class ScopedCFileDescriptor
    {
    public:
        ScopedCFileDescriptor(const char *path, const char *mode);
        ~ScopedCFileDescriptor();
        FILE * Get() const;
        bool IsValid() const;
    private:
        FILE * Descriptor;
    };
}
