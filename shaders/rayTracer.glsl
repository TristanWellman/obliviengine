/* Copyright (c) 2026 Tristan Wellman
 * DDA Screen-space Ray-tracer. Pretty much just an effecient SSR algorithm.
 * I was looking for effecient ways to cast shadows with a lot of scene lights (like Bridewell dungeon lights).
 * I came across this paper that claims "6.4 ms on a MacBook NVIDIA GeForce 650M.", so I wanted to try that.
 * https://www.jcgt.org/published/0003/04/04/paper.pdf
*/

@ctype mat4 mat4x4
@ctype vec3 vec3
@ctype vec4 vec4
@ctype vec2 vec2

@vs vs_OERT

in vec2 OERT_position;
in vec2 OERT_texcoord;
out vec2 uv;

void main() {
	uv = OERT_texcoord;
	gl_Position = vec4(OERT_position,0.0,1.0);
}

@end

@fs fs_OERT

#define NEAR 0.1
#define FAR 100.0
#define THICK 0.5
#define MAX_DIST 50.0
#define MAX_STEP 25.0

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

layout(binding=0) uniform sampler OERT_smp;
layout(binding=0) uniform texture2D OERT_texture;
layout(binding=1) uniform texture2D OERT_dtexture;
layout(binding=2) uniform texture2D OERT_ntexture;
layout(binding=3) uniform texture2D OERT_ptexture;
layout(binding=4) uniform texture2D OERT_noiseTexture;
layout(binding=5) uniform texture2D OERT_prevFrame;

layout(binding=4) uniform OERT_params {
	mat4 proj;
};

in vec2 uv;
out vec4 frag_color;

/* Grabbed from OESSGI shader
 * https://www.reedbeta.com/blog/hash-functions-for-gpu-rendering/
 * https://github.com/riccardoscalco/glsl-pcg-prng/blob/main/index.glsl
 */
uint pcg(uint v) {
	uint state = v*uint(747796405)+uint(2891336453);
	uint word = ((state>>((state>>uint(28))+uint(4)))^state)*uint(277803737);
	return (word>>uint(22))^word;
}

lowp float getRandom(vec2 p) {
	return fract(52.9829189*fract(dot(p, vec2(0.06711056, 0.00583715))));
}

vec3 traceScreenSpaceRay() {
	return vec3(1.0);
}

void main() {
	vec3 col = texture(sampler2D(OERT_texture, OERT_smp), uv).rgb;
	float zbuf = texture(sampler2D(OERT_dtexture, OERT_smp), uv).r;
	if(zbuf >= 1.0) {frag_color = vec4(col, 1.0); return;}

	frag_color = vec4(vec3(zbuf),1.0);
}

@end

@program OERT vs_OERT fs_OERT
