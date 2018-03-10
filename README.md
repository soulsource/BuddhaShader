# BuddhaShader

This small program renders a Buddhabrot ( https://en.wikipedia.org/wiki/Buddhabrot ) on the GPU, displays a preview, and allows to save the result to png (--output parameter on the command line). It does not use any advanced buddhabrot rendering techniques like importance maps or Metropolis-Hastings, and instead simply adds new points based on pseudo-random points in the complex plane. I've written this mainly to compare the rendering speed of my old implementation using only fragment shaders ( https://www.shadertoy.com/view/4ddyR2 ) to what can be achieved with modern desktop graphics APIs. The main difference to the shadertoy-code is, that this implementation uses a shader storage buffer object (SSBO) and compute shaders to operate on it. By this, every worker can compute a unique orbit, and therefore image generation is massively parallel. Zooming is not supported, as doing so would require either the implementation of importance maps, and/or of a better random number generator. Also, the fragment shader and image saving code would need to be adjusted. 

The program requires at least OpenGL 4.3, and links against libpng for export.

Many aspects of the program, including but not limited to the size of the rendered PNG and the size of the preview window, can be controlled using command line switches. Run it with the "--help" parameter to get a list.

The maximum buffer size (and therefore the maximum resolution of the generated image) is limited by graphics driver and hardware. If one requests a too big image, the program will output the maximum image size in total pixels. Similar constraints apply to work group sizes.

I doubt that this program is even big enough to fall under any copyright, but if it does, let's just say it's under zlib/libpng license.

*NOTE*: This program includes a copy of glfw ( http://www.glfw.org/ ), which is under zlib/libpng license ( http://www.glfw.org/license.html ), and a copy of files generated using GLAD ( http://glad.dav1d.de ). It further links against libpng. None of these two are my work.
