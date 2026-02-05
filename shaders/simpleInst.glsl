/* Copyright (c) 2025 Tristan Wellman
 * This is the DEFAULT instancing shader for ObliviEngine.
 * Most default things for the engine require atleast these attributes in a shader.
 * 08/30/35 - Changed phong lighting to mix in some pbr functionality.
*/


@ctype mat4 mat4x4
@ctype vec3 vec3
@ctype vec4 vec4
@ctype vec2 vec2

@block uniforms

#define MAXLIGHTS 64
#define MAXSTEPS 100
#define MAXDIST 100.0
#define EPSILON 0.001


layout(binding=0) uniform instvs_params {
	float tick;
    mat4 model;
	mat4 view;
	mat4 proj;
};

layout(binding=1) uniform instlight_params {
    vec4 positions[MAXLIGHTS]; /*[4] unused*/
    vec4 colors[MAXLIGHTS]; /*[4] is intensity*/
};

layout(binding=3) uniform instfs_params {
    vec3 camPos;
	int numLights;
};

/*These macros/functions come from include/OE/util.h*/
#define QSMAGIC 0x5f3759df
float QISQRT(float _x) {
	float y=_x;uint i=floatBitsToUint(y);
	i=(QSMAGIC-(i>>1));
	y = uintBitsToFloat(i);
	return y*(1.5-(0.5*_x*y*y));
}
float QSQRT(float _x) {
	return _x*QISQRT(_x);
}
vec3 WNORM(vec3 _x) {
	return vec3(
			_x.x*QISQRT((_x.x*_x.x)+(_x.y*_x.y)+(_x.z*_x.z)),
			_x.y*QISQRT((_x.x*_x.x)+(_x.y*_x.y)+(_x.z*_x.z)),
			_x.z*QISQRT((_x.x*_x.x)+(_x.y*_x.y)+(_x.z*_x.z)));
}

@end

@vs instvs
@include_block uniforms

in vec3 position;
in vec4 color0;
in vec3 normal0;
in vec2 texcoord0;
in vec4 instModelr0;
in vec4 instModelr1;
in vec4 instModelr2;
in vec4 instModelr3;

out float time;
out vec4 color;
out vec3 normal;
out vec3 fragPos;
out vec3 viewSpacePos;
out vec3 viewSpaceNorm;
out vec2 texcoord;

void main() {
	mat4 instModel = mat4(instModelr0, instModelr1, instModelr2, instModelr3);
	vec4 pos = instModel*vec4(position, 1.0);
    gl_Position = (proj*view)*pos;
    color = color0;

    mat3 normalMatrix = transpose(inverse(mat3(instModel)));
    normal = WNORM(normalMatrix * normal0);

    fragPos = pos.xyz;
	viewSpacePos = vec3(view*vec4(fragPos,1.0));
	viewSpaceNorm = WNORM(mat3(view) * normal);
	texcoord = texcoord0;
	time = tick;
}

@end

@fs instfs
@include_block uniforms

layout(binding=3) uniform texture2D insttexture;
layout(binding=3) uniform sampler instsmp;

in float time;
in vec4 color;
in vec3 normal; 
in vec3 fragPos;
in vec3 viewSpacePos;
in vec3 viewSpaceNorm;
in vec2 texcoord;

layout(location=0) out vec4 frag_color;
layout(location=1) out vec4 depth_color;
layout(location=2) out vec4 normal_color;
layout(location=3) out vec4 position_color;

float near = 0.1;
float far = 100.0;
float linearizeDepth(float depth) {
	float z = depth*2.0-1.0;
	return (2.0*near*far)/(far+near-z*(far-near));
}

void depth() {
	//float depth = linearizeDepth(gl_FragCoord.z)/far;
	//depth_color = vec4(vec3(depth),1.0);
	depth_color = vec4(vec3(gl_FragCoord.z),1.0);
}

void normal_c() {
	normal_color = vec4(viewSpaceNorm*0.5+0.5,1.0);
}

void position() {
	position_color = vec4(viewSpacePos,1.0);
}

/*Phong Lighting for simple shader*/
void main() {
    vec3 norm = WNORM(normal);
    vec3 viewDir = WNORM(camPos - fragPos);

    vec3 ambient;

    float shininess = 32.0;
    vec3 materialAmbient = vec3(0.1);
    vec3 materialDiffuse = vec3(0.7);
    vec3 materialSpecular = vec3(0.5);

	ambient = materialAmbient*vec3(0.02);
	/* accumulate diffuse & specular */
	vec3 ad = vec3(0.0);
	vec3 as = vec3(0.0);

	vec4 texcolor = texture(sampler2D(insttexture, instsmp), texcoord);
	
	int activeLight = clamp(numLights,0,MAXLIGHTS);
	int i;
    for(i=0;i<activeLight;i++) {
        vec3 lightPos = vec3(positions[i]);
		
		float range = positions[i].w;
		if(range<=0.001) range = 10.0;

        vec3 lightColor = colors[i].rgb;
		float intensity = colors[i].a*1.5;
		if(intensity<=0.0) intensity = 1.0;
		lightColor *= intensity;

		float d = QSQRT(dot(lightPos-fragPos,lightPos-fragPos));
		lowp float invDistSq = 1.0/max(d*d,0.01);
		lowp float rf = clamp(1.0-(d/range),0.0,1.0);
		lowp float sr = pow(rf,2.0);
		float atten = invDistSq*sr;

        vec3 lightDir = WNORM(lightPos - fragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = lightColor*materialDiffuse*diff*atten;

        float spec = pow(max(dot(norm, WNORM(lightDir+viewDir)), 0.0), shininess);
        vec3 specular = lightColor*materialSpecular*spec*atten*0.8;
		
		ad += diffuse;
		as += specular;
    }
	
	vec3 hdr = ambient+ad*texcolor.rgb+as;
	vec3 mapped = hdr/(vec3(1.0)+hdr);

    vec3 result = pow(mapped, vec3(0.45));
    frag_color = vec4(result*vec3(color), texcolor.a);

	depth();
	normal_c();
	position();
}

@end

@program simpleInst instvs instfs
