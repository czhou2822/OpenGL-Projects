#version 450


uniform vec4 light_position;
uniform vec4 light_direction;
uniform mat4 model_matrix;
uniform vec3 cameraPos;


uniform float cut_off_angle;
uniform float shininess;
uniform float refraction_index;
uniform float fog_max;
uniform float light_intensity;
uniform vec4 Kd;
uniform vec4 Ka;
uniform vec4 Ks;


uniform sampler2D texture_grass;
uniform sampler2D texture_stone;
uniform samplerCube cube_map;

uniform int texture_state;


float fog_min = 1;


in vec3 fN;
in vec3 fL;
in vec3 fE;
in vec2 texcoord;
in vec3 textureDir;

in vec3 Normal;
in vec3 Position;

out vec4  fColor;

uniform vec4 color_input;

void main() 
{	


	
	vec3 N = normalize(fN);   //normal
	vec3 E = normalize(fE);	  //eye
	vec3 L = normalize(fL);   //light
	vec3 H = normalize(L+E);  //halfway
	vec4 color_temp;
	//vec3 light_dir = Position - light_direction;

	//float angle = dot (L, normalize(-light_position).xyz);
	

	float angle = acos( dot(-L, light_direction.xyz) );
	float cutoff = radians( clamp( cut_off_angle, 0.0, 90.0 ) );

	if (angle < cutoff) {
		float spotFactor = pow ( dot (-L, light_direction.xyz),2);
		 color_temp = 	    light_intensity * Ka +
		 spotFactor * light_intensity *
		 (
		  Kd * max( dot(L, N), 0.0 ) +
		  Ks * pow(max(dot(N, H), 0.0),shininess)
		  );

	} else {
		color_temp =  light_intensity * Ka;
	}



	color_temp.a = 1.0;


	if(texture_state == 0) fColor = texture(texture_grass, texcoord);
	if(texture_state == 1) fColor = texture(texture_stone, texcoord);
	if(texture_state == 2) fColor = color_temp;
	if(texture_state == 3) fColor = vec4(0,0,1,1);
	if(texture_state == 4) fColor = vec4(0.5,0.5,0.5,1);
	if(texture_state == 5) fColor = light_intensity * Ka;
	if(texture_state == 6) fColor = vec4(1,0,0,1);

	if(texture_state == 7) {
		vec3 I = normalize(Position-cameraPos);
		vec3 R = reflect(I, normalize(Normal));
        fColor = vec4(texture(cube_map, R).rgb, 1.0);

	}

	if(texture_state == 8) {
		float ratio = 1.00 / refraction_index;
		vec3 I = normalize(Position - cameraPos);
		vec3 R = refract(I, normalize(Normal), ratio);
		fColor = vec4(texture(cube_map, R).rgb, 1.0);
	}

	float dist = abs(Position.z);
	//float fog_factor = exp(dist);
	float fog_factor = (fog_max - dist) / 
                    (fog_max - 1);
	fog_factor = clamp(fog_factor, 0.0, 1.0);
	
	
	
	
	//if(texture_state == 6) {
	
	fColor = mix(vec4(0.5,0.5,0.5,1), fColor, fog_factor);
	//}

   
} 