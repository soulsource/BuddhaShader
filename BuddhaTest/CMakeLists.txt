add_executable(
    BuddhaShader
        "src/BuddhaTest.cpp"
		"src/glad.c"
		"src/Helpers.cpp"
)

configure_file("Shaders/BuddhaFragment.glsl" ${CMAKE_CURRENT_BINARY_DIR}/Shaders/BuddhaFragment.glsl)
configure_file("Shaders/BuddhaCompute.glsl" ${CMAKE_CURRENT_BINARY_DIR}/Shaders/BuddhaCompute.glsl)
configure_file("Shaders/BuddhaVertex.glsl" ${CMAKE_CURRENT_BINARY_DIR}/Shaders/BuddhaVertex.glsl)

find_package(OpenGL REQUIRED)
find_package(PNG REQUIRED)
target_include_directories(BuddhaShader PRIVATE "include" ${OPENGL_INCLUDE_DIR} ${PNG_INCLUDE_DIRS})
target_link_libraries(BuddhaShader glfw ${OPENGL_gl_LIBRARY} ${PNG_LIBRARIES})
add_definitions(${PNG_DEFINITIONS})
add_definitions(-DINSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")
# on Linux we need to link against libdl. Maybe add id here?

install(DIRECTORY Shaders DESTINATION share/BuddhaShader FILES_MATCHING PATTERN *.glsl)
install(TARGETS BuddhaShader RUNTIME DESTINATION bin)
