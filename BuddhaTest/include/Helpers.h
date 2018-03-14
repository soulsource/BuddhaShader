#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <stdio.h> //includes FILE typedef
#include <string>

namespace Helpers
{
    GLuint LoadShaders(const std::string &vertex_file_path, const std::string &fragment_file_path);
    GLuint LoadComputeShader(const std::string &compute_file_path, unsigned int localSizeX, unsigned int localSizeY, unsigned int localSizeZ);

    bool DoesFileExist(const std::string& path);

    void WriteOutputPNG(const std::string& path, const std::vector<uint32_t>& data, unsigned int width, unsigned int bufferHeight, double gamma, double colorScale);

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

    struct RenderSettings
    {
        unsigned int imageWidth = 1024;
        unsigned int imageHeight = 576;

        unsigned int windowWidth = 1024;
        unsigned int windowHeight = 576;

        unsigned int orbitLengthRed = 10;
        unsigned int orbitLengthGreen = 100;
        unsigned int orbitLengthBlue = 1000;

        unsigned int localWorkgroupSizeX = 1024;
        unsigned int localWorkgroupSizeY = 1;
        unsigned int localWorkgroupSizeZ = 1;

        unsigned int globalWorkGroupSizeX = 1024;
        unsigned int globalWorkGroupSizeY = 1;
        unsigned int globalWorkGroupSizeZ = 1;

        unsigned int iterationsPerFrame = 1000;

        std::string pngFilename = "";
        double pngGamma = 1.0;
        double pngColorScale = 2.0;

        unsigned int ignoreMaxBufferSize = 0;

        bool CheckValidity();
        bool ParseCommandLine(int argc, char * argv[]);
    };
}
