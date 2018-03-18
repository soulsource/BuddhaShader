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

        unsigned int orbitLengthSkip = 0;
        unsigned int orbitLengthRed = 10;
        unsigned int orbitLengthGreen = 100;
        unsigned int orbitLengthBlue = 1000;

        unsigned int localWorkgroupSizeX = 16;
        unsigned int localWorkgroupSizeY = 16;
        unsigned int localWorkgroupSizeZ = 1;

        unsigned int globalWorkGroupSizeX = 64;
        unsigned int globalWorkGroupSizeY = 64;
        unsigned int globalWorkGroupSizeZ = 1;

        unsigned int targetFrameRate = 60;

        std::string pngFilename = "";
        double pngGamma = 1.0;
        double pngColorScale = 2.0;

        unsigned int ignoreMaxBufferSize = 0;
        unsigned int printDebugOutput = 0;

        bool CheckValidity();
        bool ParseCommandLine(int argc, char * argv[]);
    };

    template<typename ValueType, typename TimeType>
    class PIDController
    {
    public:
        PIDController(ValueType prop, ValueType diff, ValueType integral) : propFactor(prop), diffFactor(diff), intFactor(integral) {}

        ValueType Update(TimeType dT, ValueType Error)
        {
            errorSum += Error * dT;
            const auto differential{(Error - lastError)/dT};
            lastError = Error;
            return Error * propFactor + errorSum * intFactor + differential * diffFactor;
        }

    protected:
        ValueType propFactor{};
        ValueType diffFactor{};
        ValueType intFactor{};

    private:
        ValueType errorSum{};
        ValueType lastError{};
    };
}
