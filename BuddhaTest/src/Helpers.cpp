#include "Helpers.h"
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <png.h>

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

	GLuint LoadComputeShader(const char* compute_file_path)
	{
		GLuint ComputeShaderID = glCreateShader(GL_COMPUTE_SHADER);
		// Read the compute shader
		std::string ComputeShaderCode;
		{
			std::ifstream ComputeShaderCodeStream(compute_file_path, std::ios::in);
			if (ComputeShaderCodeStream.is_open()) {
				std::stringstream sstr;
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
    void WriteOutputPNG(const std::vector<uint32_t>& data, unsigned int width, unsigned int height)
    {
        png_byte ** row_pointers = static_cast<png_byte **>(calloc(height,sizeof(void *)));
        for(int i = 0; i < height ; ++i)
        {
            row_pointers[i] = static_cast<png_byte *>(calloc(3*width,sizeof(png_byte)));
        }

        uint32_t maxValue{UINT32_C(0)};
        for(unsigned int i = 0; i < data.size();++i)
        {
            maxValue = std::max(maxValue,data[i]);
        }
        for(unsigned int i = 0; i < data.size();++i)
        {
            unsigned int row = (i)/(3*width);
            unsigned int col = i - 3*row*width;
            row_pointers[row][col] = (255*data[i] + (maxValue/2))/maxValue;
        }

        //beware, this is going to be ugly C syntax, but well, libpng...
        FILE * fp = fopen("image.png", "wb");
        if(!fp)
        {
            return;
        }
        png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if(!png_ptr)
        {
            fclose(fp);
            return;
        }
        png_infop info_ptr = png_create_info_struct(png_ptr);
        if(!info_ptr)
        {
            png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
            fclose(fp);
            return;
        }
        if(setjmp(png_jmpbuf(png_ptr)))
        {
            png_destroy_write_struct(&png_ptr, &info_ptr);
            fclose(fp);
            return;
        }
        png_init_io(png_ptr, fp);
        png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

        png_write_info(png_ptr, info_ptr);
        //header written.

        png_write_image(png_ptr, row_pointers);

        png_write_end(png_ptr, info_ptr);
        png_destroy_write_struct(&png_ptr, &info_ptr);

        for(int i = 0; i < height ; ++i)
        {
            free(row_pointers[i]);
        }

        free(row_pointers);
    }
}
