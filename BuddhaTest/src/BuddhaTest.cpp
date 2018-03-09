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
    unsigned int bufferWidth = 640;
    unsigned int bufferHeight = 480;


	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	glfwSetErrorCallback(error_callback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Buddhabrot", NULL, NULL);
	if (!window)
	{
		std::cerr << "Failed to create OpenGL 4.3 core context. We do not support compatibility contexts." << std::endl;
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

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
        const unsigned int pixelCount{(bufferWidth * bufferHeight)*3}; //*3 -> RGB
        const unsigned int integerCount = pixelCount + 2; //first two elements in buffer: width/height.
        std::vector<uint32_t> initializeBuffer(integerCount,0);
        initializeBuffer[0] = bufferWidth;
        initializeBuffer[1] = bufferHeight;
        glBufferData(GL_SHADER_STORAGE_BUFFER, 4 * integerCount, initializeBuffer.data(), GL_DYNAMIC_COPY);
    }
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, drawBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// Create and compile our GLSL program from the shaders
	GLuint VertexAndFragmentShaders = Helpers::LoadShaders("Shaders/BuddhaVertex.glsl", "Shaders/BuddhaFragment.glsl");
	//Do the same for the compute shader:
	GLuint ComputeShader = Helpers::LoadComputeShader("Shaders/BuddhaCompute.glsl");

    uint32_t iterationCount{0};
    glUseProgram(ComputeShader);
    GLint iterationCountUniformHandle = glGetUniformLocation(ComputeShader, "iterationCount");

    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		//let the compute shader do something
		glUseProgram(ComputeShader);
        //increase iterationCount, which is used for pseudo random generation
        glUniform1ui(iterationCountUniformHandle,++iterationCount);
		glDispatchCompute(1920, 1080, 1);

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
		// Draw the triangle !
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // Triangle strip with 4 vertices -> quad.
		glDisableVertexAttribArray(0);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
