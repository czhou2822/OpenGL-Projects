#version 450

uniform mat4 projection_matrix;
uniform mat4 model_matrix;
uniform mat4 view;
//in vec4 normal;
uniform float coefficient;

layout (location = 0) in  vec3 vPosition;
layout (location = 1) in vec3 normal;

//in  vec4 vColor;
out vec4 interpolatedColor;

void main() 
{
  // PROTIP #3
  // can extend to gl_Position = projection * camera * models * vertex
  // you may wish to view camera motion as a transform on the projection (projection * camera) * models ...
  // this could be thought of as moving the observer in space
  // or you may view camera motion as transforming all objects in the world while the observer remains still
  // projection * (camera * models) * ...
  // notice mathematically there is no difference
  // however in rare circumstances the order of the transform may affect the numerical stability
  // of the overall projection
  gl_Position = projection_matrix*view*model_matrix*((coefficient*vec4(normal, 0.0))+vec4(vPosition, 1.0f));
  interpolatedColor = vec4(normal, 1.0f);
} 
