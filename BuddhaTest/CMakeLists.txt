add_executable(
    BuddhaTest 
        "src/BuddhaTest.cpp"
		"src/glad.c"
		"src/Helpers.cpp"
)

configure_file("Shaders/BuddhaFragment.glsl" ${CMAKE_CURRENT_BINARY_DIR}/Shaders/BuddhaFragment.glsl)
configure_file("Shaders/BuddhaCompute.glsl" ${CMAKE_CURRENT_BINARY_DIR}/Shaders/BuddhaCompute.glsl)
configure_file("Shaders/BuddhaVertex.glsl" ${CMAKE_CURRENT_BINARY_DIR}/Shaders/BuddhaVertex.glsl)

find_package(OpenGL REQUIRED)
target_include_directories(BuddhaTest PRIVATE "include" ${OPENGL_INCLUDE_DIR})
target_link_libraries(BuddhaTest glfw ${OPENGL_gl_LIBRARY})
# on Linux we need to link against libdl. Maybe add id here?