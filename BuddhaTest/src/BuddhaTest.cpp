#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Helpers.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <cmath>

void error_callback(int error, const char* description)
{
    std::cerr << "Error: " << description << std::endl;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main(int argc, char * argv[])
{
    Helpers::RenderSettings settings;

    if(!settings.ParseCommandLine(argc,argv))
    {
        return 2;
    }



    float imageRangeMinX = -2.0f;
    float imageRangeMinY = -1.125f;
    float imageRangeMaxX = 2.0f;
    float imageRangeMaxY = 1.125f;





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

    //register callback on window resize:
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    //disable vsync
    glfwSwapInterval(0);

    //we have a context. Let's check if input is sane.
    //calcualte buffer size, and make sure it's allowed by the driver.
    const unsigned int pixelCount{(settings.imageWidth * settings.imageHeight)};
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
        std::cerr << "Something went wrong with loading the shaders. Abort." << std::endl;
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
    glBufferData(GL_SHADER_STORAGE_BUFFER, 4 *3* pixelCount, nullptr, GL_DYNAMIC_COPY);
    glClearBufferData(GL_SHADER_STORAGE_BUFFER,GL_R8,GL_RED,GL_UNSIGNED_INT,nullptr);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, drawBuffer);

    GLuint brightnessBuffer;
    glGenBuffers(1,&brightnessBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, brightnessBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 4,nullptr, GL_DYNAMIC_COPY);
    glClearBufferData(GL_SHADER_STORAGE_BUFFER,GL_R8,GL_RED,GL_UNSIGNED_INT,nullptr);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, brightnessBuffer);

    GLuint importanceMapBuffer;
    glGenBuffers(1,&importanceMapBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER,importanceMapBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER,4*(100*50),nullptr, GL_DYNAMIC_COPY); //100*50 pixels. Map is symmetric around x-axis. This size is *hardcoded* in the shader!
    glClearBufferData(GL_SHADER_STORAGE_BUFFER,GL_R8,GL_RED,GL_UNSIGNED_INT,nullptr);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, importanceMapBuffer);

    //the state buffer is special. While making each entry 8 bytes large and using std430 layout, the data is never read back,
    //so we can get away with being more lenient and allowing the compiler to choose layout without much extra work.
    //We need to query the size for allocation though.
    GLuint stateBufferIndex = glGetProgramResourceIndex(ComputeShader,GL_SHADER_STORAGE_BLOCK,"statusBuffer");
    GLint bufferAlignment;
    GLenum t = GL_BUFFER_DATA_SIZE;
    glGetProgramResourceiv(ComputeShader,GL_SHADER_STORAGE_BLOCK,stateBufferIndex,1,&t,1,nullptr,&bufferAlignment);
    GLuint stateStructIndex = glGetProgramResourceIndex(ComputeShader,GL_BUFFER_VARIABLE,"stateArray[0].phase");
    GLint requiredStateBufferSizePerWorker;
    t = GL_TOP_LEVEL_ARRAY_STRIDE;
    glGetProgramResourceiv(ComputeShader,GL_BUFFER_VARIABLE,stateStructIndex,1,&t,1,nullptr,&requiredStateBufferSizePerWorker);

    const uint32_t workersPerFrame = settings.globalWorkGroupSizeX*settings.globalWorkGroupSizeY*settings.globalWorkGroupSizeZ*settings.localWorkgroupSizeX*settings.localWorkgroupSizeY*settings.localWorkgroupSizeZ;
    auto requiredStateMemory = ((requiredStateBufferSizePerWorker*workersPerFrame + bufferAlignment -1)/bufferAlignment) * bufferAlignment;
    GLuint stateBuffer;
    glGenBuffers(1,&stateBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER,stateBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, (requiredStateMemory),nullptr,GL_DYNAMIC_COPY);
    glClearBufferData(GL_SHADER_STORAGE_BUFFER,GL_R8,GL_RED,GL_UNSIGNED_INT,nullptr);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, stateBuffer);

    glUseProgram(ComputeShader);
    GLint orbitLengthUniformHandle = glGetUniformLocation(ComputeShader, "orbitLength");
    GLint totalIterationsUniformHandle = glGetUniformLocation(ComputeShader, "totalIterations");
    GLint widthUniformComputeHandle = glGetUniformLocation(ComputeShader, "width");
    GLint heightUniformComputeHandle = glGetUniformLocation(ComputeShader, "height");
    GLint iterationsPerDispatchHandle = glGetUniformLocation(ComputeShader, "iterationsPerDispatch");
    GLint imageRangeComputeHandle = glGetUniformLocation(ComputeShader,"effectiveImageRange");
    glUniform4ui(orbitLengthUniformHandle,settings.orbitLengthRed,settings.orbitLengthGreen,settings.orbitLengthBlue,settings.orbitLengthSkip);
    glUniform1ui(widthUniformComputeHandle, settings.imageWidth);
    glUniform1ui(heightUniformComputeHandle, settings.imageHeight);
    float effectiveImageRange[4];
    effectiveImageRange[0] = imageRangeMinX;
    effectiveImageRange[2] = imageRangeMaxX;
    if(imageRangeMinY*imageRangeMaxY < 0)
    {
        effectiveImageRange[1] = std::max(imageRangeMinY,-imageRangeMaxY);
        effectiveImageRange[3] = std::max(-imageRangeMinY,imageRangeMaxY);
    }
    else
    {
        effectiveImageRange[1] = std::min(std::abs(imageRangeMinY),std::abs(imageRangeMaxY));
        effectiveImageRange[3] = std::max(std::abs(imageRangeMinY),std::abs(imageRangeMaxY));
    }
    glUniform4f(imageRangeComputeHandle,effectiveImageRange[0],effectiveImageRange[1],effectiveImageRange[2],effectiveImageRange[3]);

    const uint32_t maxOrbitlength = std::max(std::max(settings.orbitLengthBlue,settings.orbitLengthGreen),settings.orbitLengthRed);
    glUniform1ui(totalIterationsUniformHandle, maxOrbitlength);

    glUseProgram(VertexAndFragmentShaders);
    GLint widthUniformFragmentHandle = glGetUniformLocation(VertexAndFragmentShaders, "width");
    GLint heightUniformFragmentHandle = glGetUniformLocation(VertexAndFragmentShaders, "height");
    GLint gammaUniformFragmentHandle = glGetUniformLocation(VertexAndFragmentShaders, "gamma");
    GLint colorScaleUniformFragmentHandle = glGetUniformLocation(VertexAndFragmentShaders, "colorScale");
    glUniform1ui(widthUniformFragmentHandle, settings.imageWidth);
    glUniform1ui(heightUniformFragmentHandle, settings.imageHeight);
    glUniform1f(gammaUniformFragmentHandle, float(settings.pngGamma));
    glUniform1f(colorScaleUniformFragmentHandle,float(settings.pngColorScale));

    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    uint32_t iterationsPerFrame = 1;

    Helpers::PIDController<float, std::chrono::high_resolution_clock::time_point::rep> pid{0.0f,0.0f,1e-4f};
    const uint32_t targetFrameDuration{1000000/settings.targetFrameRate};

    uint64_t totalIterationCount{0};
    uint64_t lastMessage{0};

    const auto startTime{std::chrono::high_resolution_clock::now()};
    auto frameStop{startTime};
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window) && (settings.benchmarkTime == 0 || std::chrono::duration_cast<std::chrono::seconds>(frameStop-startTime).count() < settings.benchmarkTime))
    {
        auto frameStart{std::chrono::high_resolution_clock::now()};
        totalIterationCount += iterationsPerFrame;
        //let the compute shader do something
        glUseProgram(ComputeShader);
        glUniform1ui(iterationsPerDispatchHandle, iterationsPerFrame);
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
        frameStop = std::chrono::high_resolution_clock::now();
        const auto dur{std::chrono::duration_cast<std::chrono::microseconds>(frameStop-frameStart)};
        auto frameDuration{dur.count()};
        if(frameDuration > 0)
        {
            const auto error{targetFrameDuration - frameDuration};
            const auto pidOutput{pid.Update(1,float(error))};
            iterationsPerFrame = std::max(1,static_cast<int>(pidOutput));

            //std::cout << iterationsPerFrame << " " << pidOutput << std::endl;
        }
        if(settings.printDebugOutput != 0 && totalIterationCount/maxOrbitlength > lastMessage)
        {
            lastMessage = totalIterationCount/maxOrbitlength;
            const auto ctime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::cout << "Iteration count next frame: " << iterationsPerFrame << std::endl;
            std::cout << std::put_time(std::localtime(&ctime),"%X") << ": Iteration count per worker higher than: " << lastMessage*maxOrbitlength << std::endl;
            std::cout << std::put_time(std::localtime(&ctime),"%X") << ": Total iteration count higher than: " << lastMessage*maxOrbitlength*workersPerFrame << std::endl;
        }
    }

    //settings.pngFilename = "Don'tForgetToRemoveThisLine.png";
    if(!settings.pngFilename.empty() || settings.benchmarkTime != 0)
    {
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        std::vector<uint32_t> readBackBuffer(pixelCount*3);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, drawBuffer);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER,0,4 *3* pixelCount,readBackBuffer.data());

        if(settings.benchmarkTime != 0)
            Helpers::PrintBenchmarkScore(readBackBuffer);

        if(!settings.pngFilename.empty())
            Helpers::WriteOutputPNG(settings.pngFilename,readBackBuffer,settings.imageWidth,settings.imageHeight, settings.pngGamma, settings.pngColorScale);
    }

    //a bit of cleanup
    glDeleteBuffers(1,&vertexbuffer);
    glDeleteBuffers(1,&drawBuffer);
    glDeleteBuffers(1,&stateBuffer);

    glfwTerminate();
    return 0;
}
