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
#define RADIUS 0.5
#define BIAS 0.025

layout(binding=4) uniform OESSAO_params {
	vec4 kernel[MAXKSIZE];
	mat4x4 proj;
};

layout(binding=0) uniform sampler OESSAO_smp;
layout(binding=0) uniform texture2D OESSAO_texture;
layout(binding=1) uniform texture2D OESSAO_dtexture;
layout(binding=2) uniform texture2D OESSAO_ntexture;
layout(binding=3) uniform texture2D OESSAO_ptexture;


in vec2 uv;
out vec4 frag_color;

void main() {
	vec4 col = texture(sampler2D(OESSAO_texture, OESSAO_smp), uv);
	vec3 pcol = texture(sampler2D(OESSAO_ptexture, OESSAO_smp), uv).xyz;
	vec3 ncol = texture(sampler2D(OESSAO_ntexture, OESSAO_smp), uv).xyz;

	vec3 up = abs(ncol.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 t = normalize(cross(up,ncol));
	vec3 b = cross(ncol, t);
	mat3 TBN = mat3(t,b,ncol);

	float AO = 0.0;
	int i = 0;
	for(;i<MAXKSIZE;i++) {
		vec3 samplePos = TBN*kernel[i].xyz;
		samplePos = pcol+samplePos*RADIUS;
		vec4 offs = proj*vec4(samplePos, 1.0);
		offs.xyz /= offs.w;
		vec2 suv = offs.xy*0.5+0.5;
		if(suv.x<0.0||suv.x>1.0||suv.y<0.0||suv.y>1.0) continue;
		float depth = texture(sampler2D(OESSAO_dtexture, OESSAO_smp), suv).r;
		float rangeCheck = smoothstep(0.0,1.0,RADIUS/abs(pcol.z-depth));
		if(depth>=samplePos.z+BIAS) AO += rangeCheck;
	}
	AO = 1.0-(AO/MAXKSIZE);
	frag_color = vec4(vec3(AO),1.0)*col;
	//frag_color = vec4(ncol,1.0);
}

@end

@program OESSAO vs_OESSAO fs_OESSAO
