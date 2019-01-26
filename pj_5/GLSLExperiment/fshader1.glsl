#version 450



uniform mat4 model_matrix;
uniform vec3 cameraPos;
uniform sampler2D texture_og;  //0
uniform sampler2D mandlebrot;  //1
uniform int texture_state;
uniform float uT;  //tolerance
uniform float uQuantize;  //tolerance
in vec2 texcoord;
const float pi = 3.14159265;

vec2 st = texcoord;

ivec2 ires = textureSize(texture_og, 0);
float res_s = float(ires.s);
float res_t = float(ires.t);
vec3 irgb = texture(texture_og, texcoord).rgb;

vec2 stp0 = vec2 (1./res_s,  0       );
vec2 st0p = vec2 (0.      ,  1./res_t);
vec2 stpp = vec2 (1./res_s,  1./res_t);
vec2 stpm = vec2 (1./res_s, -1./res_t);
const vec3 w = vec3(0.2125, 0.7154, 0.0721);

out vec4  fColor;

vec4 get_negative(vec4 input_color){
	return vec4( 1-input_color.x, 1-input_color.y, 1-input_color.z, 1);
}

float get_mag(){
	float i00   = dot( texture(texture_og, texcoord)     .rgb, w);
	float im1m1 = dot( texture(texture_og, texcoord-stpp).rgb, w);
	float ip1p1 = dot( texture(texture_og, texcoord+stpp).rgb, w);
	float im1p1 = dot( texture(texture_og, texcoord-stpm).rgb, w);
	float ip1m1 = dot( texture(texture_og, texcoord+stpm).rgb, w);
	float im10  = dot( texture(texture_og, texcoord-stp0).rgb, w);
	float ip10  = dot( texture(texture_og, texcoord+stp0).rgb, w);
	float i0m1  = dot( texture(texture_og, texcoord-st0p).rgb, w);
	float i0p1  = dot( texture(texture_og, texcoord+st0p).rgb, w);
	
	float h = -1.*im1p1 - 2.*i0p1 - 1.*ip1p1 + 1.*im1m1 + 2.*i0m1 + 1.*ip1m1; 
	float v = -1.*im1m1 - 2.*im10 - 1.*im1p1 + 1.*ip1m1 + 2.*ip10 + 1.*ip1p1; 

	return length (vec2 (h,v));

}

vec4 get_edge_detection(){

	float mag = get_mag();

	vec3 target = vec3 (mag, mag, mag);

	return vec4 (mix (irgb, target, uT), 1);
}

vec4 get_toon(vec4 input_color){
	vec4 col = texture(texture_og, texcoord);
	float mag = get_mag();
	if(mag > uT) return vec4(0,0,0,1);
	else {
		col.rgb *= uQuantize;
		col.rgb += vec3(.5, .5, .5);
		ivec3 intrgb = ivec3 (col.rgb);
		col.rgb = vec3(intrgb) / uQuantize; 
		return vec4(col.rgb, 1);
	}
}

vec4 get_embossing(){

	vec3 c00 =   texture (texture_og, texcoord).rgb;
	vec3 cp1p1 = texture (texture_og, texcoord+stpp).rgb;
	vec3 diffs = c00-cp1p1;
	float max = diffs.r;
	if( abs(diffs.g) > abs(max) ) max = diffs.g;
	if( abs(diffs.b) > abs(max) ) max = diffs.b;

	float gray = clamp (max + .5, 0., 1. );
	vec3 color = vec3 (gray, gray, gray);
	return vec4 (color, 1);
	
}

vec4 get_twirl(){


	float radius = res_s * 0.7;
	vec2 xy = res_s * st;

	vec2 dxy = xy - res_s/2;
	float r = length (dxy);
	float beta = atan (dxy.y, dxy.x) + radians(60) * (radius-r)/radius;
	
	vec2 xy1 = xy;
	if(r <= radius ) xy1 = res_s/2 + r*vec2(cos(beta), sin(beta));

	st = xy1/res_s;
	vec3 irgb = texture (texture_og, st).rgb;
	return vec4(irgb, 1);
}

vec4 get_ripple(){
	float taux = 120;
	float tauy = 250;
	float alphax = 10;
	float alphay = 15;

	ivec2 og_coord = ivec2(texcoord.s * res_s, texcoord.t * res_s);

	float x_new = og_coord.s+alphax*sin((2*pi*og_coord.t)/taux);
	float y_new = og_coord.t+alphay*sin((2*pi*og_coord.s)/tauy);

	vec2 new_st = vec2 (x_new/res_s, y_new/res_s);
	vec3 not_output =   texture (texture_og, new_st).rgb;

	return vec4 (not_output, 1);
	
}

vec4 get_spherical(){
	float rho = 1.8;

	ivec2 og_coord = ivec2(texcoord.s * res_s, texcoord.t * res_s);

	float x_new = og_coord.s;
	float y_new = og_coord.t;

	float radius = 0.5 * res_s;

	vec2 center = 0.5 * ires;
	vec2 dxy = og_coord - center;
	float r = length (dxy);
	float z = sqrt(pow(radius,2) - pow(r,2));

	float beta_x = (1-1/rho) * asin(dxy.s/sqrt(pow(dxy.s, 2)+pow(z, 2)));
	float beta_y = (1-1/rho) * asin(dxy.t/sqrt(pow(dxy.t, 2)+pow(z, 2)));

	if (r <= radius){
		x_new = og_coord.s - z*tan(beta_x);
		y_new = og_coord.y - z*tan(beta_y);
	} 

	
	vec2 new_st = vec2 (x_new/res_s, y_new/res_t);
	vec3 not_output =   texture (texture_og, new_st).rgb;

	return vec4 (not_output, 1);
	
}

void main() 
{	

	
	
	vec4 color_temp = texture(texture_og, texcoord);

	//luminance

	float luminance = dot (irgb, w);


	if     (texture_state == 0) fColor = color_temp; 
	else if(texture_state == 1) fColor = vec4( luminance, luminance, luminance, 1);
	else if(texture_state == 2) fColor = get_negative(color_temp);
	else if(texture_state == 3) fColor = get_edge_detection();
	else if(texture_state == 4) fColor = get_embossing();
	else if(texture_state == 5) fColor = get_toon(color_temp); 
	//else if(texture_state == 5) fColor = vec4(1.0, 0.0, 0.0, 1.0);
	else if(texture_state == 6) fColor = get_twirl();
	else if(texture_state == 7) fColor = get_ripple(); 
	else if(texture_state == 8) fColor = get_spherical(); 
	else if(texture_state == 9) fColor = texture(mandlebrot, texcoord); 


	else fColor = texture(texture_og, texcoord); 

   


} 

