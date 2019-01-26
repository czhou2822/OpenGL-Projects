#version 450


uniform mat4 projection_matrix;


layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec3 normal;



out vec2 texcoord;


void main() 
{

  gl_Position = projection_matrix*vPosition;
  texcoord = vPosition.xy;
  
} 
