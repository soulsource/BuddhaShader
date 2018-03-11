#include "Helpers.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <png.h>
#include <cmath>
#include <unordered_map>
#include <string>
#include <algorithm>

namespace Helpers
{
    GLuint LoadShaders(const std::string& vertex_file_path, const std::string& fragment_file_path) {

		// Create the shaders
		GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
		GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

		// Read the Vertex Shader code from the file
		std::string VertexShaderCode;
		std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
		if (VertexShaderStream.is_open()) {
			std::stringstream sstr;
			sstr << VertexShaderStream.rdbuf();
			VertexShaderCode = sstr.str();
			VertexShaderStream.close();
		}
		else {
            std::cerr << "Failed to load vertex shader file from " << vertex_file_path << ". Is this installed correctly?" << std::endl;
			return 0;
		}

		// Read the Fragment Shader code from the file
		std::string FragmentShaderCode;
		std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
		if (FragmentShaderStream.is_open()) {
			std::stringstream sstr;
			sstr << FragmentShaderStream.rdbuf();
			FragmentShaderCode = sstr.str();
			FragmentShaderStream.close();
		}
        else
        {
            std::cerr << "Failed to load fragment shader file from " << fragment_file_path << ". Is this installed correctly?" << std::endl;
            return 0;
        }

		GLint Result = GL_FALSE;
		int InfoLogLength;

		// Compile Vertex Shader
        //std::cout << "Compiling shader : " << vertex_file_path << std::endl;
		char const * VertexSourcePointer = VertexShaderCode.c_str();
		glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
		glCompileShader(VertexShaderID);

		// Check Vertex Shader
		glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0) {
			std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
			glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
			printf("%s\n", &VertexShaderErrorMessage[0]);
		}


		// Compile Fragment Shader
        //std::cout << "Compiling shader : " << fragment_file_path << std::endl;
		char const * FragmentSourcePointer = FragmentShaderCode.c_str();
		glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
		glCompileShader(FragmentShaderID);

		// Check Fragment Shader
		glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0) {
			std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
			glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
			printf("%s\n", &FragmentShaderErrorMessage[0]);
		}

		// Link the program
        //printf("Linking program\n");
		GLuint ProgramID = glCreateProgram();
		glAttachShader(ProgramID, VertexShaderID);
		glAttachShader(ProgramID, FragmentShaderID);
		glLinkProgram(ProgramID);

		// Check the program
		glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
		glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0) {
			std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
			glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
			printf("%s\n", &ProgramErrorMessage[0]);
		}

		glDetachShader(ProgramID, VertexShaderID);
		glDetachShader(ProgramID, FragmentShaderID);

		glDeleteShader(VertexShaderID);
		glDeleteShader(FragmentShaderID);

		return ProgramID;
	}

    GLuint LoadComputeShader(const std::string& compute_file_path, unsigned int localSizeX, unsigned int localSizeY, unsigned int localSizeZ)
	{
		GLuint ComputeShaderID = glCreateShader(GL_COMPUTE_SHADER);
		// Read the compute shader
		std::string ComputeShaderCode;
		{
			std::ifstream ComputeShaderCodeStream(compute_file_path, std::ios::in);
			if (ComputeShaderCodeStream.is_open()) {
				std::stringstream sstr;
                sstr << "#version 430" <<
                        std::endl <<
                        "layout (local_size_x = " << localSizeX <<
                        ", local_size_y = " << localSizeY <<
                        ", local_size_z = " << localSizeZ << ") in;" << std::endl;
				sstr << ComputeShaderCodeStream.rdbuf();
				ComputeShaderCode = sstr.str();
				ComputeShaderCodeStream.close();
			}
            else
            {
                std::cerr << "Failed to load compute shader file from " << compute_file_path << ". Is this installed correctly?" << std::endl;
                return 0;
            }
		}

		GLint Result = GL_FALSE;
		int InfoLogLength;

		{
			// Compile Compute Shader
            //std::cout << "Compiling shader : " << compute_file_path << std::endl;
			char const * ComputeSourcePointer = ComputeShaderCode.c_str();
			glShaderSource(ComputeShaderID, 1, &ComputeSourcePointer, NULL);
			glCompileShader(ComputeShaderID);

			// Check Compute Shader
			glGetShaderiv(ComputeShaderID, GL_COMPILE_STATUS, &Result);
			glGetShaderiv(ComputeShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
			if (InfoLogLength > 0) {
				std::vector<char> ComputeShaderErrorMessage(InfoLogLength + 1);
				glGetShaderInfoLog(ComputeShaderID, InfoLogLength, NULL, &ComputeShaderErrorMessage[0]);
				printf("%s\n", &ComputeShaderErrorMessage[0]);
			}
		}
		GLuint ProgramID = glCreateProgram();
		glAttachShader(ProgramID, ComputeShaderID);
		glLinkProgram(ProgramID);

		// Check the program
		glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
		glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0) {
			std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
			glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
			printf("%s\n", &ProgramErrorMessage[0]);
		}
		glDetachShader(ProgramID, ComputeShaderID);
		glDeleteShader(ComputeShaderID);
		return ProgramID;
	}

    void WriteOutputPNG(const std::string &path, const std::vector<uint32_t>& data, unsigned int width, unsigned int bufferHeight, double gamma, double colorScale)
    {
        std::vector<png_byte> pngData(3*width*2*bufferHeight);
        std::vector<png_byte *> rows{2*bufferHeight};
        for(unsigned int i = 0; i < 2*bufferHeight ; ++i)
        {
            rows[i] = pngData.data()+3*width*i;
        }

        uint32_t maxValue{UINT32_C(1)};
        for(unsigned int i = 0; i < data.size();++i)
        {
            maxValue = std::max(maxValue,data[i]);
        }
        for(unsigned int i = 0; i < data.size();++i)
        {
            if(fabs(gamma - 1.0) > 0.0001 || fabs(colorScale - 1.0) > 0.0001)
            {
                pngData[data.size() + i] = static_cast<png_byte>(255.0 * pow(std::min(1.0,colorScale*static_cast<double>(data[i])/static_cast<double>(maxValue)),gamma));
            }
            else
            {
                pngData[data.size() + i] = (255*data[i] + (maxValue/2))/maxValue;
            }
        }
        for(unsigned int i = 0; i < bufferHeight;++i)
        {
            for(unsigned int j = 0; j < width*3;++j)
            {
                rows[i][j] =rows[2*bufferHeight-i-1][j];
            }
        }

        ScopedCFileDescriptor fd(path.c_str(), "wb");
        if(!fd.IsValid())
        {
            std::cerr << "Failed to open image.png for writing." << std::endl;
            return;
        }
        png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if(!png_ptr)
        {
            return;
        }
        png_infop info_ptr = png_create_info_struct(png_ptr);
        if(!info_ptr)
        {
            png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
            return;
        }
        if(setjmp(png_jmpbuf(png_ptr)))
        {
            png_destroy_write_struct(&png_ptr, &info_ptr);
            return;
        }
        png_init_io(png_ptr, fd.Get());
        png_set_IHDR(png_ptr, info_ptr, width, 2*bufferHeight, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

        png_write_info(png_ptr, info_ptr);
        //header written.

        png_write_image(png_ptr, rows.data());

        png_write_end(png_ptr, info_ptr);
        png_destroy_write_struct(&png_ptr, &info_ptr);
    }

    ScopedCFileDescriptor::ScopedCFileDescriptor(const char *path, const char *mode)
    {
        Descriptor = fopen(path,mode);
    }

    ScopedCFileDescriptor::~ScopedCFileDescriptor()
    {
        if(IsValid())
            fclose(Descriptor);
    }

    FILE * ScopedCFileDescriptor::Get() const
    {
        return Descriptor;
    }

    bool ScopedCFileDescriptor::IsValid() const
    {
        return Descriptor != nullptr;
    }

    bool RenderSettings::CheckValidity()
    {
        if(imageHeight%2 != 0)
        {
            std::cerr << "Image height has to be an even number." << std::endl;
            return false;
        }
        const unsigned int bufferHeight = imageHeight/2;
        const unsigned int pixelCount{(imageWidth * imageHeight/2)*3}; //*3 -> RGB
        int maxSSBOSize;
        glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE,&maxSSBOSize);
        if(pixelCount * 4 > static_cast<unsigned int>(maxSSBOSize))
        {
            std::cerr << "Requested buffer size larger than maximum allowed by graphics driver. Max pixel number: " << maxSSBOSize/6 << std::endl;
            return false;
        }
        int WorkGroupSizeLimitX, WorkGroupSizeLimitY, WorkGroupSizeLimitZ;
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT,0,&WorkGroupSizeLimitX);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT,1,&WorkGroupSizeLimitY);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT,2,&WorkGroupSizeLimitZ);
        if(globalWorkGroupSizeX > static_cast<unsigned int>(WorkGroupSizeLimitX) ||
                globalWorkGroupSizeY > static_cast<unsigned int>(WorkGroupSizeLimitY) ||
                globalWorkGroupSizeZ > static_cast<unsigned int>(WorkGroupSizeLimitZ))
        {
            std::cerr << "Requested global work group size exceeds maximum dimension. Limits: " << WorkGroupSizeLimitX << ", " << WorkGroupSizeLimitY << ", " << WorkGroupSizeLimitZ << std::endl;
            return false;
        }
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE,0,&WorkGroupSizeLimitX);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE,1,&WorkGroupSizeLimitY);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE,2,&WorkGroupSizeLimitZ);
        if(localWorkgroupSizeX > static_cast<unsigned int>(WorkGroupSizeLimitX) ||
                localWorkgroupSizeY > static_cast<unsigned int>(WorkGroupSizeLimitY) ||
                localWorkgroupSizeZ > static_cast<unsigned int>(WorkGroupSizeLimitZ))
        {
            std::cerr << "Requested local work group size exceeds maximum dimension. Limits: " << WorkGroupSizeLimitX << ", " << WorkGroupSizeLimitY << ", " << WorkGroupSizeLimitZ << std::endl;
            return false;
        }
        int maxInvocations;
        glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS,&maxInvocations);
        if(localWorkgroupSizeX*localWorkgroupSizeY*localWorkgroupSizeZ > static_cast<unsigned int>(maxInvocations))
        {
            std::cerr << "Requested local work group size exceeds maximum total count (Product of x*y*z dimensions). Limit: " << maxInvocations << std::endl;
            return false;
        }
        return true;
    }

    bool RenderSettings::ParseCommandLine(int argc, char *argv[])
    {
        struct SettingsPointer
        {
        public:
            SettingsPointer(unsigned int * ptr) : intPtr(ptr), dblPtr(nullptr), stringPtr(nullptr) {}
            SettingsPointer(double * ptr) : intPtr(nullptr), dblPtr(ptr), stringPtr(nullptr) {}
            SettingsPointer(std::string * ptr) : intPtr(nullptr), dblPtr(nullptr), stringPtr(ptr) {}
            unsigned int * GetIntPtr() const { return intPtr; }
            double * GetDblPtr() const { return dblPtr;}
            std::string * GetStringPtr() const { return stringPtr;}
        private:
            unsigned int * intPtr = nullptr;
            double * dblPtr = nullptr;
            std::string * stringPtr = nullptr;
        };
        std::unordered_map<std::string, SettingsPointer> commandMap{
            {"--imageWidth",&imageWidth},
            {"--imageHeight",&imageHeight},
            {"--windowWidth",&windowWidth},
            {"--windowHeight",&windowHeight},
            {"--orbitLengthRed",&orbitLengthRed},
            {"--orbitLengthGreen",&orbitLengthGreen},
            {"--orbitLengthBlue",&orbitLengthBlue},
            {"--localWorkgroupSizeX", &localWorkgroupSizeX},
            {"--localWorkgroupSizeY", &localWorkgroupSizeY},
            {"--localWorkgroupSizeZ", &localWorkgroupSizeZ},
            {"--globalWorkgroupSizeX", &globalWorkGroupSizeX},
            {"--globalWorkgroupSizeY", &globalWorkGroupSizeY},
            {"--globalWorkgroupSizeZ", &globalWorkGroupSizeZ},
            {"--imageGamma",&pngGamma},
            {"--imageColorScale",&pngColorScale},
            {"--output", &pngFilename}
        };

        for(int i=1; i < argc;++i)
        {
            std::string argAsString(argv[i]);
            if(argAsString == "--help") //help is a special case.
            {
                std::cout << "Draws a buddhabrot and iterates until the user closes the window. If a --output filename is given, a png file will afterwards be written there." <<std::endl <<
                             "Supported options are:" << std::endl << std::endl <<
                             "--output [path] : File to write output to. Empty by default, meaning no output is written." << std::endl <<
                             "--imageWidth [integer] : Width of the to be written image. 1024 by default. If no --output is given, this still detrmines the buffer size for rendering." << std::endl <<
                             "--imageHeight [integer] : Height of the to be written image. 576 by default. If no --output is given, this still detrmines the buffer size for rendering." << std::endl <<
                             "--imageGamma [float] : Gamma to use when writing the image. 1.0 by default. Ignored if no --output is given." << std::endl <<
                             "--imageColorScale [float] : Image brightness is scaled by the brightest pixel. The result is multiplied by this value. 2.0 by default, as 1.0 leaves very little dynamic range." << std::endl <<
                             "--windowWidth [integer] : Width of the preview window. 1024 by default." << std::endl <<
                             "--windowHeight [integer] : Height of the preview window. 576 by default." << std::endl <<
                             "--orbitLengthRed [integer] : Maximum number of iterations for escaping orbits to color red. 10 by default." << std::endl <<
                             "--orbitLengthGreen [integer] : Maximum number of iterations for escaping orbits to color green. 100 by default." << std::endl <<
                             "--orbitLengthBlue [integer] : Maximum number of iterations for escaping orbits to color blue. 1000 by default." << std::endl <<
                             "--localWorkgroupSizeX [integer] : How \"parallel\" the computation should be. Maximum possible value depends on GPU and drivers. The default is 1024. Values up to 1024 are guaranteed to work." << std::endl <<
                             "--localWorkgroupSizeY [integer] : How \"parallel\" the computation should be. Maximum possible value depends on GPU and drivers. The default is 1. Values up to 1024 are guaranteed to work." << std::endl <<
                             "--localWorkgroupSizeZ [integer] : How \"parallel\" the computation should be. Maximum possible value depends on GPU and drivers. The default of 1. Values up to 1024 are guaranteed to work." << std::endl <<
                             "\tNOTE: There's also a limit on the product of the three local workgroup sizes, for which a number smaller or equal to 1024 is guaranteed to work. Higher numbers might work and run faster. Feel free to experiment." << std::endl <<
                             "--globalWorkgroupSizeX [integer] : How often the local work group should be invoked per frame. Values up to 65535 are guaranteed to work. Default is 1024." << std::endl <<
                             "--globalWorkgroupSizeY [integer] : How often the local work group should be invoked per frame. Values up to 65535 are guaranteed to work. Default is 1." << std::endl <<
                             "--globalWorkgroupSizeZ [integer] : How often the local work group should be invoked per frame. Values up to 65535 are guaranteed to work. Default is 1." << std::endl;
                return false;
            }
        }
        //apart from --help all parameters consist of 2 entries.
        if(argc%2 != 1)
        {
            std::cerr << "Invalid number of arguments. See --help for usage." << std::endl;
            return false;
        }
        for(int i=1 ; i < argc; i += 2)
        {
            std::string argAsString(argv[i]);
            std::string valueAsString(argv[i+1]);
            auto setting = commandMap.find(argAsString);
            if(setting == commandMap.end())
            {
                std::cerr << "Unknown option: " << argAsString << std::endl;
                return false;
            }
            const SettingsPointer& ptr = setting->second;
            if(auto intProp = ptr.GetIntPtr())
            {
                *intProp = std::stoi(valueAsString);
            }
            else if(auto dblProp = ptr.GetDblPtr())
            {
                *dblProp = std::stod(valueAsString);
            }
            else if(auto strPtr = ptr.GetStringPtr())
            {
                *strPtr = valueAsString;
            }
            else
            {
                std::cerr << "Something went horribly wrong with command line parsing. This is a bug." << std::endl;
                return false;
            }
        }

        return true;
    }

    bool DoesFileExist(const std::string &path)
    {
        std::ifstream f(path);
            return f.good();
    }

}
