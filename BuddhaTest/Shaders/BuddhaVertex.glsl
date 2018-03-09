#version 430 core

layout(location = 0) in vec3 vertexPosition_modelspace;

out vec2 uv;

void main(){
  gl_Position.xyz = vertexPosition_modelspace;
  gl_Position.w = 1.0;
  uv = gl_Position.xy;
}