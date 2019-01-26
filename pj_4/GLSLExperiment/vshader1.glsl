#version 450

uniform vec4 light_position;
uniform mat4 projection_matrix;
uniform mat4 model_matrix;
uniform mat4 view;



layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec3 normal;



out vec3 fN;
out vec3 fE;
out vec3 fL;

out vec3 Normal;
out vec3 Position;

out vec2 texcoord;
out vec3 textureDir;

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

  vec4 fN1 = model_matrix*normalize(vec4(normal, 0.0));
  //vec4 fN1 = normalize(vec4(normal, 1.0));
  fN = fN1.xyz;
  fE = -vPosition.xyz;
  fL = light_position.xyz - vPosition.xyz;

  //if(light_position.w!=0){
  //fL = light_position.xyz - vPosition.xyz;
  //}

  
  Normal = fN1.xyz;
  Position = (model_matrix * vPosition).xyz;

  gl_Position = projection_matrix*view*model_matrix*vPosition;
  texcoord = vPosition.xy;
  textureDir = vPosition.xyz;
 
  
} 
