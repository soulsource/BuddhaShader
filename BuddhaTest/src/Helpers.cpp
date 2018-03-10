#include "Helpers.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <png.h>
#include <cmath>

namespace Helpers
{
	GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path) {

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
			printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
			getchar();
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

		GLint Result = GL_FALSE;
		int InfoLogLength;

		// Compile Vertex Shader
		printf("Compiling shader : %s\n", vertex_file_path);
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
		printf("Compiling shader : %s\n", fragment_file_path);
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
		printf("Linking program\n");
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

    GLuint LoadComputeShader(const char* compute_file_path, unsigned int localSizeX, unsigned int localSizeY, unsigned int localSizeZ)
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
		}

		GLint Result = GL_FALSE;
		int InfoLogLength;

		{
			// Compile Compute Shader
			printf("Compiling shader : %s\n", compute_file_path);
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

    void WriteOutputPNG(const std::vector<uint32_t>& data, unsigned int width, unsigned int bufferHeight, double gamma, double colorScale)
    {
        std::vector<png_byte> pngData(3*width*2*bufferHeight);
        std::vector<png_byte *> rows{2*bufferHeight};
        for(int i = 0; i < 2*bufferHeight ; ++i)
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
                pngData[data.size() + i] = 255.0 * pow(std::min(1.0,colorScale*static_cast<double>(data[i])/static_cast<double>(maxValue)),gamma);
            }
            else
            {
                pngData[data.size() + i] = (255*data[i] + (maxValue/2))/maxValue;
            }
        }
        for(int i = 0; i < bufferHeight;++i)
        {
            for(int j = 0; j < width*3;++j)
            {
                rows[i][j] =rows[2*bufferHeight-i-1][j];
            }
        }

        ScopedCFileDescriptor fd("image.png", "wb");
        if(!fd.IsValid())
        {
            std::cout << "Failed to open image.png for writing." << std::endl;
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

}
