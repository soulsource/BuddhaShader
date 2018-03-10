#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Helpers.h>
#include <iostream>
#include <vector>

void error_callback(int error, const char* description)
{
	std::cerr << "Error: " << description << std::endl;
}

int main(int argc, char * argv[])
{
    Helpers::RenderSettings settings;

    if(!settings.ParseCommandLine(argc,argv))
    {
        return 2;
    }

    unsigned int bufferHeight = settings.imageHeight/2;

	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	glfwSetErrorCallback(error_callback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(settings.windowWidth, settings.windowHeight, "Buddhabrot", NULL, NULL);
	if (!window)
	{
		std::cerr << "Failed to create OpenGL 4.3 core context. We do not support compatibility contexts." << std::endl;
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    //we have a context. Let's check if input is sane.
    //calcualte buffer size, and make sure it's allowed by the driver.
    const unsigned int pixelCount{(settings.imageWidth * bufferHeight)*3}; //*3 -> RGB
    if(!settings.CheckValidity())
    {
        glfwTerminate();
        return 1;
    }

    // Create and compile our GLSL program from the shaders
    std::string vertexPath("Shaders/BuddhaVertex.glsl");
    std::string fragmentPath("Shaders/BuddhaFragment.glsl");
    std::string computePath("Shaders/BuddhaCompute.glsl");

    if(!Helpers::DoesFileExist(vertexPath))
    {
#if defined _WIN32 || defined __CYGWIN__
        std::string separator("\\");
#else
        std::string separator("/");
#endif
        vertexPath = INSTALL_PREFIX + separator + "share" + separator + "BuddhaShader" + separator + vertexPath;
        fragmentPath = INSTALL_PREFIX + separator + "share" + separator + "BuddhaShader" + separator + fragmentPath;
        computePath = INSTALL_PREFIX + separator + "share" + separator + "BuddhaShader" + separator + computePath;
    }

    GLuint VertexAndFragmentShaders = Helpers::LoadShaders(vertexPath, fragmentPath);
    //Do the same for the compute shader:
    GLuint ComputeShader = Helpers::LoadComputeShader(computePath, settings.localWorkgroupSizeX, settings.localWorkgroupSizeY, settings.localWorkgroupSizeZ);
    if(VertexAndFragmentShaders == 0 || ComputeShader == 0)
    {
        std::cout << "Something went wrong with loading the shaders. Abort." << std::endl;
        glfwTerminate();
        return 1;
    }

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	const GLfloat g_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		1.0f, 1.0f, 0.0f
	};

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);


	GLuint drawBuffer;
	glGenBuffers(1, &drawBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, drawBuffer);
    {
        glBufferData(GL_SHADER_STORAGE_BUFFER, 4 * pixelCount, nullptr, GL_DYNAMIC_COPY);
        glClearBufferData(GL_SHADER_STORAGE_BUFFER,GL_R8,GL_RED,GL_UNSIGNED_INT,nullptr);
    }
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, drawBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    uint32_t iterationCount{0};
    glUseProgram(ComputeShader);
    GLint iterationCountUniformHandle = glGetUniformLocation(ComputeShader, "iterationCount");
    GLint orbitLengthUniformHandle = glGetUniformLocation(ComputeShader, "orbitLength");
    GLint widthUniformComputeHandle = glGetUniformLocation(ComputeShader, "width");
    GLint heightUniformComputeHandle = glGetUniformLocation(ComputeShader, "height");
    glUniform3ui(orbitLengthUniformHandle,settings.orbitLengthRed,settings.orbitLengthGreen,settings.orbitLengthBlue);
    glUniform1ui(widthUniformComputeHandle, settings.imageWidth);
    glUniform1ui(heightUniformComputeHandle, bufferHeight);

    glUseProgram(VertexAndFragmentShaders);
    GLint widthUniformFragmentHandle = glGetUniformLocation(VertexAndFragmentShaders, "width");
    GLint heightUniformFragmentHandle = glGetUniformLocation(VertexAndFragmentShaders, "height");
    glUniform1ui(widthUniformFragmentHandle, settings.imageWidth);
    glUniform1ui(heightUniformFragmentHandle, bufferHeight);

    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		//let the compute shader do something
		glUseProgram(ComputeShader);
        //increase iterationCount, which is used for pseudo random generation
        glUniform1ui(iterationCountUniformHandle,++iterationCount);
        glDispatchCompute(settings.globalWorkGroupSizeX, settings.globalWorkGroupSizeY, settings.globalWorkGroupSizeZ);

        //before reading the values in the ssbo, we need a memory barrier:
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); //I hope this is the correct (and only required) bit

		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(VertexAndFragmentShaders);
		
        glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);
        // Draw the triangle strip!
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // Triangle strip with 4 vertices -> quad.
		glDisableVertexAttribArray(0);

		/* Swap front and back buffers */
        glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

    if(!settings.pngFilename.empty())
    {
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, drawBuffer);
        {
            std::vector<uint32_t> readBackBuffer(pixelCount);
            glGetBufferSubData(GL_SHADER_STORAGE_BUFFER,0,4 * pixelCount,readBackBuffer.data());
            Helpers::WriteOutputPNG(settings.pngFilename,readBackBuffer,settings.imageWidth,bufferHeight, settings.pngGamma, settings.pngColorScale);
        }
    }

    //a bit of cleanup
    glDeleteBuffers(1,&vertexbuffer);
    glDeleteBuffers(1,&drawBuffer);

	glfwTerminate();
	return 0;
}
