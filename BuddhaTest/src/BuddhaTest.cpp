#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Helpers.h>
#include <iostream>
#include <vector>

void error_callback(int error, const char* description)
{
	std::cerr << "Error: " << description << std::endl;
}

int main()
{
    unsigned int bufferWidth = 6304;
    unsigned int bufferHeight = 1773;

    unsigned int windowWidth = 1600;
    unsigned int windowHeight = 900;

    unsigned int orbitLengthRed = 10;
    unsigned int orbitLengthGreen = 100;
    unsigned int orbitLengthBlue = 1000;

    unsigned int localWorkgroupSizeX = 1024;
    unsigned int localWorkgroupSizeY = 1;
    unsigned int localWorkgroupSizeZ = 1;

    unsigned int globalWorkGroupSizeX = 1024;
    unsigned int globalWorkGroupSizeY = 1;
    unsigned int globalWorkGroupSizeZ = 1;

    double pngGamma = 1.0;
    double pngColorScale = 2.0;

	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	glfwSetErrorCallback(error_callback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(windowWidth, windowHeight, "Buddhabrot", NULL, NULL);
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
    const unsigned int pixelCount{(bufferWidth * bufferHeight)*3}; //*3 -> RGB
    {
        int maxSSBOSize;
        glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE,&maxSSBOSize);
        if(pixelCount * 4 > maxSSBOSize)
        {
            std::cout << "Requested buffer size larger than maximum allowed by graphics driver. Max pixel number: " << maxSSBOSize/12 << std::endl;
            glfwTerminate();
            return 0;
        }
        int WorkGroupSizeLimitX, WorkGroupSizeLimitY, WorkGroupSizeLimitZ;
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT,0,&WorkGroupSizeLimitX);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT,1,&WorkGroupSizeLimitY);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT,2,&WorkGroupSizeLimitZ);
        if(globalWorkGroupSizeX > WorkGroupSizeLimitX || globalWorkGroupSizeY > WorkGroupSizeLimitY || globalWorkGroupSizeZ > WorkGroupSizeLimitZ)
        {
            std::cout << "Requested global work group size exceeds maximum dimension. Limits: " << WorkGroupSizeLimitX << ", " << WorkGroupSizeLimitY << ", " << WorkGroupSizeLimitZ << std::endl;
            glfwTerminate();
            return 0;
        }
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE,0,&WorkGroupSizeLimitX);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE,1,&WorkGroupSizeLimitY);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE,2,&WorkGroupSizeLimitZ);
        if(localWorkgroupSizeX > WorkGroupSizeLimitX || localWorkgroupSizeY > WorkGroupSizeLimitY || localWorkgroupSizeZ > WorkGroupSizeLimitZ)
        {
            std::cout << "Requested local work group size exceeds maximum dimension. Limits: " << WorkGroupSizeLimitX << ", " << WorkGroupSizeLimitY << ", " << WorkGroupSizeLimitZ << std::endl;
            glfwTerminate();
            return 0;
        }
        int maxInvocations;
        glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS,&maxInvocations);
        if(localWorkgroupSizeX*localWorkgroupSizeY*localWorkgroupSizeZ > maxInvocations)
        {
            std::cout << "Requested local work group size exceeds maximum total count (Product of x*y*z dimensions). Limit: " << maxInvocations << std::endl;
            glfwTerminate();
            return 0;
        }
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

	// Create and compile our GLSL program from the shaders
	GLuint VertexAndFragmentShaders = Helpers::LoadShaders("Shaders/BuddhaVertex.glsl", "Shaders/BuddhaFragment.glsl");
	//Do the same for the compute shader:
    GLuint ComputeShader = Helpers::LoadComputeShader("Shaders/BuddhaCompute.glsl", localWorkgroupSizeX, localWorkgroupSizeY, localWorkgroupSizeZ);

    uint32_t iterationCount{0};
    glUseProgram(ComputeShader);
    GLint iterationCountUniformHandle = glGetUniformLocation(ComputeShader, "iterationCount");
    GLint orbitLengthUniformHandle = glGetUniformLocation(ComputeShader, "orbitLength");
    GLint widthUniformComputeHandle = glGetUniformLocation(ComputeShader, "width");
    GLint heightUniformComputeHandle = glGetUniformLocation(ComputeShader, "height");
    glUniform3ui(orbitLengthUniformHandle,orbitLengthRed,orbitLengthGreen,orbitLengthBlue);
    glUniform1ui(widthUniformComputeHandle, bufferWidth);
    glUniform1ui(heightUniformComputeHandle,bufferHeight);

    glUseProgram(VertexAndFragmentShaders);
    GLint widthUniformFragmentHandle = glGetUniformLocation(VertexAndFragmentShaders, "width");
    GLint heightUniformFragmentHandle = glGetUniformLocation(VertexAndFragmentShaders, "height");
    glUniform1ui(widthUniformFragmentHandle, bufferWidth);
    glUniform1ui(heightUniformFragmentHandle,bufferHeight);

    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		//let the compute shader do something
		glUseProgram(ComputeShader);
        //increase iterationCount, which is used for pseudo random generation
        glUniform1ui(iterationCountUniformHandle,++iterationCount);
        glDispatchCompute(globalWorkGroupSizeX, globalWorkGroupSizeY, globalWorkGroupSizeZ);

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

    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, drawBuffer);
    {
        std::vector<uint32_t> readBackBuffer(pixelCount);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER,0,4 * pixelCount,readBackBuffer.data());
        Helpers::WriteOutputPNG(readBackBuffer,bufferWidth,bufferHeight, pngGamma, pngColorScale);
    }

    //a bit of cleanup
    glDeleteBuffers(1,&vertexbuffer);
    glDeleteBuffers(1,&drawBuffer);

	glfwTerminate();
	return 0;
}
