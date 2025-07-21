/*
	Copyright (c) 2025 Tristan Wellman 
	Obliviengine SSAO Post-Pass Shader.
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

#define MAXKSIZE 128
#define KSIZE 128
#define RADIUS 0.25
#define BIAS 0.05
#define SCALE 43758.5453

/*These come from ssgi.glsl*/
#define QSMAGIC 0x5f3759df
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

layout(binding=4) uniform OESSAO_params {
	vec4 kernel[MAXKSIZE];
	mat4x4 proj;
};

layout(binding=0) uniform sampler OESSAO_smp;
layout(binding=0) uniform texture2D OESSAO_texture;
layout(binding=2) uniform texture2D OESSAO_ntexture;
layout(binding=3) uniform texture2D OESSAO_ptexture;


in vec2 uv;
out vec4 frag_color;

float getRandom(vec2 uv) {
	return fract(sin(dot(uv,vec2(12.98,78.23)))*SCALE);
}

void main() {
	vec3 col = texture(sampler2D(OESSAO_texture, OESSAO_smp), uv).rgb;
	vec3 pcol = texture(sampler2D(OESSAO_ptexture, OESSAO_smp), uv).xyz;
	if(length(pcol)<0.001) {frag_color=vec4(col,1.0);return;}
	vec3 ncol = WNORM(texture(sampler2D(OESSAO_ntexture, OESSAO_smp), uv).xyz*2.0-1.0);

	vec3 up = abs(ncol.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 t = WNORM(cross(up,ncol));
	vec3 b = cross(ncol, t);
	mat3 TBN = mat3(t,b,ncol);

	float AO = 0.0;
	int i = 0;
	for(;i<KSIZE;i++) {
		vec3 samplePos = TBN*kernel[i].xyz;
		samplePos = pcol+samplePos*RADIUS;
		vec4 clip = proj*vec4(samplePos,1.0);
		vec2 suv = (clip.xy/clip.w)*0.5+0.5;
		if(any(lessThan(suv,vec2(0.0)))||any(greaterThan(suv,vec2(1.0)))) continue;
		vec3 hitPos = texture(sampler2D(OESSAO_ptexture, OESSAO_smp), suv).xyz;
		if(length(hitPos)<0.001) continue;
		float rangeCheck = smoothstep(0.0,RADIUS,abs(pcol.z-hitPos.z));
		AO += (hitPos.z<=samplePos.z+BIAS?1.0:0.0)*(rangeCheck);
	}
	AO = 1.0-(AO/float(KSIZE));
	frag_color = vec4(col*AO,1.0);
}

@end

@program OESSAO vs_OESSAO fs_OESSAO
