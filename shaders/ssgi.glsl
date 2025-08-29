/* Copyright (c) 2025 Tristan Wellman
 * Screen Space Global Illumination Post-Pass Shader.
 * A lot of this is copied from the ssao shader.
 */

@ctype mat4 mat4x4
@ctype vec3 vec3
@ctype vec4 vec4
@ctype vec2 vec2

@vs vs_OESSGI

in vec2 OESSGI_position;
in vec2 OESSGI_texcoord;
out vec2 uv;

void main() {
	uv = OESSGI_texcoord;
	gl_Position = vec4(OESSGI_position,0.0,1.0);
}

@end

@fs fs_OESSGI

/*#define RAYS 16
#define STEPS 64*/
#define RADIUS 0.25
#define BIAS 0.05
#define INTENSITY 2.0
#define PI 3.1415926
#define SCALE 43758.5453

/*These macros come from include/OE/util.h*/
#define QSMAGIC 0x5f3759df
/* C-style macros not work :(
 * #define QISQRT(_x) \
	({float y=(_x);uint32_t i=*(uint32_t*)&y; \
	  i=(QSMAGIC-(i>>1));y=*(float*)&i; \
	  y*=(1.5f-(0.5f*(_x)*y*y));y;})
#define WNORM(_x) ((vec3(_x)) { \
	(_x.x)*QISQRT((_x.x*_x.x)+(_x.y*_x.y)+(_x.z*_x.z)), \
	(_x.y)*QISQRT((_x.x*_x.x)+(_x.y*_x.y)+(_x.z*_x.z)), \
	(_x.z)*QISQRT((_x.x*_x.x)+(_x.y*_x.y)+(_x.z*_x.z))})*/

float QISQRT(float _x) {
	float y=_x;uint i=floatBitsToUint(y);
	i=(QSMAGIC-(i>>1));
	return y*(1.5-(0.5*_x*y*y));
}
vec3 WNORM(vec3 _x) {
	return vec3(
			_x.x*QISQRT((_x.x*_x.x)+(_x.y*_x.y)+(_x.z*_x.z)),
			_x.y*QISQRT((_x.x*_x.x)+(_x.y*_x.y)+(_x.z*_x.z)),
			_x.z*QISQRT((_x.x*_x.x)+(_x.y*_x.y)+(_x.z*_x.z)));
}

layout(binding=4) uniform OESSGI_params {
	mat4 proj;
	/*
	 * low: rays = 16 steps = 64
	 * med: rays = 32 steps = 128
	 * high: rays = 64 steps = 256
	 * */
	int RAYS; 
	int STEPS;
};

layout(binding=0) uniform sampler OESSGI_smp;
layout(binding=0) uniform texture2D OESSGI_texture;
layout(binding=2) uniform texture2D OESSGI_ntexture;
layout(binding=3) uniform texture2D OESSGI_ptexture;
layout(binding=4) uniform texture2D OESSGI_noiseTexture;
layout(binding=5) uniform texture2D OESSGI_prevFrame;

in vec2 uv;
out vec4 frag_color;

float getRandom(vec2 uv) {
	return fract(sin(dot(uv,vec2(12.98,78.23)))*SCALE);
}

void main() {
	vec3 col = texture(sampler2D(OESSGI_texture, OESSGI_smp), uv).rgb;
	vec3 pcol = texture(sampler2D(OESSGI_ptexture, OESSGI_smp), uv).xyz;
	if(length(pcol)<0.001) {frag_color=vec4(col,1.0);return;}
	vec3 ncol = WNORM(texture(sampler2D(OESSGI_ntexture, OESSGI_smp), uv).xyz*2.0-1.0);

	vec3 up = abs(ncol.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 t = WNORM(cross(up,ncol));
	vec3 b = cross(ncol, t);
	mat3 TBN = mat3(t,b,ncol);
	vec3 GI = vec3(0.0);
	float seed = getRandom(uv*pcol.z);

	float invSteps = 1.0/float(STEPS);
	float tpi = PI*2;

	int i=0, hits=0;
	for(;i<RAYS;i++) {
		float phi = tpi*getRandom(vec2(i,seed));
		float cosT = getRandom(vec2(seed,i));
		float sinT = sqrt(1.0-cosT*cosT);
		vec3 dirVS = vec3(cos(phi)*sinT, sin(phi)*sinT, cosT);
		vec3 rayDir = TBN*dirVS;
		int j = 1;
		for(;j<=STEPS;j++) {
			float ta = (float(j)*invSteps)*RADIUS;
			vec3 samplePos = pcol+rayDir*ta;
			vec4 clip = proj*vec4(samplePos,1.0);
			vec2 suv = (clip.xy/clip.w)*0.5+0.5;
			if(any(lessThan(suv,vec2(0.0)))||any(greaterThan(suv,vec2(1.0)))) continue;
			vec3 hitPos = texture(sampler2D(OESSGI_ptexture, OESSGI_smp), suv).xyz;
			if(length(hitPos)<0.001) continue;
			if(hitPos.z<samplePos.z-BIAS) {
				vec3 hitColor = texture(sampler2D(OESSGI_texture, OESSGI_smp), suv).rgb;
				float wei = max(dot(ncol,rayDir), 0.0)*(1.0-ta/RADIUS);
				GI += hitColor*wei; 
				hits++;
				break;
			}
		}
	}
	GI = (GI/float(RAYS))*INTENSITY;
	float AO = ((float(hits)/float(RAYS)));
	AO = clamp(AO,0.2,1.0);
	frag_color = vec4((col+GI)*AO,1.0);
}

@end

@program OESSGI vs_OESSGI fs_OESSGI

