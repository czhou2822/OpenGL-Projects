#version 450

in  vec4  interpolatedColor;
out vec4  fColor;

uniform vec4 color_input;

void main() 
{ 
    fColor = interpolatedColor;
	fColor = vec4(1.0, 0.0, 0.0, 1.0);
	fColor = color_input;
} 

