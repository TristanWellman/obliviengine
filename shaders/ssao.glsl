/* Copyright (c) 2025 Tristan Wellman 
 * Obliviengine SSAO Post-Pass Shader.
 *
 * This isn't exactly SSAO if you may notice.
 * With a single pass, high radius, and light greyscale blending 
 * mixed behind the SSGI shader you get fake shadows.
 */


@ctype mat4 mat4x4
@ctype vec3 vec3
@ctype vec4 vec4
@ctype vec2 vec2

@vs vs_OESSAO

in vec2 OESSAO_position;
in vec2 OESSAO_texcoord;
out vec2 uv;

void main() {
	uv = OESSAO_texcoord;
	gl_Position = vec4(OESSAO_position,0.0,1.0);
}

@end

@fs fs_OESSAO

#define RADIUS 0.55
#define BIAS 0.05
#define INTENSITY 2.0
#define PI 3.1415926

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

layout(binding=4) uniform OESSAO_params {
	mat4 proj;
};

layout(binding=0) uniform sampler OESSAO_smp;
layout(binding=0) uniform texture2D OESSAO_texture;
layout(binding=2) uniform texture2D OESSAO_ntexture;
layout(binding=3) uniform texture2D OESSAO_ptexture;
layout(binding=4) uniform texture2D OESSAO_noiseTexture;

in vec2 uv;
out vec4 frag_color;

/* 
 * https://www.reedbeta.com/blog/hash-functions-for-gpu-rendering/
 * https://github.com/riccardoscalco/glsl-pcg-prng/blob/main/index.glsl
 */
uint pcg(uint v) {
	uint state = v*uint(747796405)+uint(2891336453);
	uint word = ((state>>((state>>uint(28))+uint(4)))^state)*uint(277803737);
	return (word>>uint(22))^word;
}

lowp float getRandom(vec2 p) {
	return float(pcg(pcg(uint(p.x))+uint(p.y)))/float(uint(0xffffffff));
}

void main() {
	vec3 col = texture(sampler2D(OESSAO_texture, OESSAO_smp), uv).rgb;
	vec3 pcol = texture(sampler2D(OESSAO_ptexture, OESSAO_smp), uv).xyz;
	if(QSQRT(dot(pcol,pcol))<0.001) {frag_color=vec4(col,1.0);return;}
	vec3 ncol = texture(sampler2D(OESSAO_ntexture, OESSAO_smp), uv).xyz*2.0-1.0;

	lowp vec3 up = abs(ncol.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	lowp vec3 t = cross(up,ncol);
	lowp vec3 b = cross(ncol, t);
	lowp mat3 TBN = mat3(t,b,ncol);
	lowp float seed = getRandom(uv*pcol.x*pcol.y*pcol.z);

	lowp float tpi = PI*2;
	lowp float hits = 0.6;

	lowp float phi = tpi*getRandom(vec2(0.0,seed));
	lowp float cosT = getRandom(vec2(seed,0.0));
	lowp float sinT = (1.0-cosT*cosT)*QISQRT(1.0-cosT*cosT);
	lowp vec3 dirVS = vec3(cos(phi)*sinT, sin(phi)*sinT, cosT);
	vec3 rayDir = TBN*dirVS;
	lowp float ta = RADIUS;
	lowp vec3 samplePos = pcol+rayDir*ta;
	lowp vec4 clip = proj*vec4(samplePos,1.0);
	lowp vec2 suv = (clip.xy/clip.w)*0.5+0.5;
	lowp vec3 hitPos = texture(sampler2D(OESSAO_ptexture, OESSAO_smp), suv).xyz;
	if(hitPos.z<samplePos.z-BIAS) {hits=1.0;}
	lowp vec3 sum = col.rgb*hits;
	frag_color = vec4(sum,1.0);
}

@end

@program OESSAO vs_OESSAO fs_OESSAO

