/* Copyright (c) 2025 Tristan Wellman
 * Based on this beauty here: https://github.com/BrutPitt/glslSmartDeNoise
 * */

@ctype mat4 mat4x4
@ctype vec3 vec3
@ctype vec4 vec4
@ctype vec2 vec2

@vs vs_OEDNOISE

in vec2 OEDNOISE_position;
in vec2 OEDNOISE_texcoord;
out vec2 uv;

void main() {
	uv = OEDNOISE_texcoord;
	gl_Position = vec4(OEDNOISE_position,0.0,1.0);
}

@end

@fs fs_OEDNOISE

#define INVSQRT2PI 0.3989422804014326
#define INVPI 0.31830988618379
#define SIGMA 2.0
#define KSIGMA 2.0
#define THRESH 0.1

layout(binding=1) uniform OEDNOISE_params {
	vec2 resolution;
};

layout(binding=0) uniform sampler OEDNOISE_smp;
layout(binding=0) uniform texture2D OEDNOISE_texture;

in vec2 uv;
out vec4 frag_color;

void smartDeNoise(vec3 col, vec2 uv, 
		float sigma, float ksigma, float thresh) {

}

void main() {
	vec4 col = texture(sampler2D(OEDNOISE_texture, OEDNOISE_smp), uv);
	float radius = round(KSIGMA*SIGMA);
	float radQ = pow(radius,2.0);

	float invSigmaQx2 = 0.5/pow(SIGMA,2.0);
	float invSigmaQx2Pi = INVPI/THRESH;
	float invThreshSqx2 = 0.5/pow(THRESH,2);
	float invThreshSqrt2Pi = INVSQRT2PI/THRESH;

	float zBuff = 0.0;
	vec4 aBuff = vec4(0.0);
	vec2 d;
	for(d.x=-radius;d.x<=radius;d.x++) {
		float pt = sqrt(radQ-d.x*d.x);
		for(d.y=-pt;d.y<=pt;d.y++) {
			float blur = exp(-dot(d,d)*invSigmaQx2)*invSigmaQx2Pi;
			vec4 walkPx = texture(sampler2D(OEDNOISE_texture, OEDNOISE_smp), 
					uv+d/resolution);
			vec4 dC = walkPx-col;
			float delta = exp(-dot(dC.rgb,dC.rgb)*invThreshSqx2)*invThreshSqrt2Pi*blur;
			zBuff += delta;
			aBuff += delta*walkPx;
		}
	}
	frag_color = aBuff/zBuff;
}

@end

@program OEDNOISE vs_OEDNOISE fs_OEDNOISE

