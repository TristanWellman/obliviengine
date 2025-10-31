/* Copyright (c) 2025 Tristan Wellman
 * This is the optimized low-end phong shader for OE.
 * Most default things for the engine require atleast these attributes in a shader.
 */


@ctype mat4 mat4x4
@ctype vec3 vec3
@ctype vec4 vec4
@ctype vec2 vec2

@block uniforms

#define MAXLIGHTS 64

#define MATDIFF vec3(0.7)
#define AMB vec3(0.002)

layout(binding=0) uniform OELOW_vs_params {
    mat4 mvp;
    mat4 model;
	mat4 view;
};

layout(binding=1) uniform OELOW_light_params {
    vec4 positions[MAXLIGHTS]; /*[4] unused*/
    vec4 colors[MAXLIGHTS]; /*[4] is intensity*/
};

layout(binding=3) uniform OELOW_fs_params {
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

@vs OELOW_vs
@include_block uniforms

in vec3 position;
in vec4 color0;
in vec3 normal0;
in vec2 texcoord0;

out lowp vec4 color;
out lowp vec3 normal;
out lowp vec3 fragPos;
out lowp vec3 viewSpacePos;
out lowp vec3 viewSpaceNorm;
out lowp vec2 texcoord;

void main() {
    gl_Position = mvp * vec4(position,1.0);
    color = color0;

    lowp mat3 normalMatrix = transpose(inverse(mat3(model)));
    normal = WNORM(normalMatrix * normal0);

    fragPos = vec3(model * vec4(position,1.0));
	viewSpacePos = vec3(view*vec4(fragPos,1.0));
	viewSpaceNorm = WNORM(mat3(view) * normal);
	texcoord = texcoord0;
}

@end

@fs OELOW_fs
@include_block uniforms

layout(binding=3) uniform texture2D _texture;
layout(binding=3) uniform sampler smp;

in lowp vec4 color;
in lowp vec3 normal; 
in lowp vec3 fragPos;
in lowp vec3 viewSpacePos;
in lowp vec3 viewSpaceNorm;
in lowp vec2 texcoord;

layout(location=0) out lowp vec4 frag_color;
layout(location=1) out lowp vec4 depth_color;
layout(location=2) out lowp vec4 normal_color;
layout(location=3) out lowp vec4 position_color;

void depth() {
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
    lowp vec3 norm = normal;
    lowp vec3 viewDir = WNORM(camPos - fragPos);

	vec3 ad = vec3(0.0);

	lowp vec3 texcolor = texture(sampler2D(_texture, smp), texcoord).rgb;
	
	int al = clamp(numLights,0,MAXLIGHTS);
	int i;
    for(i=0;i<al;i++) {
		lowp float d = QSQRT(dot(vec3(positions[i])-fragPos,
				vec3(positions[i])-fragPos));
		lowp float atten = 1.0/(d*d);

        lowp vec3 lightDir = WNORM(vec3(positions[i]) - fragPos);
        lowp float diff = max(dot(norm, lightDir), 0.0);
        lowp vec3 diffuse = 
			vec3(colors[i]*(colors[i].a*1.5))*MATDIFF*diff*atten;
		ad += diffuse;
    }
	lowp vec3 hdr = (AMB+ad*texcolor);
    frag_color = vec4(pow(hdr/(vec3(1.0)+hdr),vec3(0.5))*vec3(color), 1.0);

	depth();
	normal_c();
	position();
}

@end

@program simple_low OELOW_vs OELOW_fs
