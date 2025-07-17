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
#define RADIUS 0.5
#define BIAS 0.05

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

/*https://www.shadertoy.com/view/Ms33WB*/
#define MOD3 vec3(.1031,.11369,.13787)
float hash12(vec2 p) {
	vec3 p3  = fract(vec3(p.xyx) * MOD3);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}
float getRandom(vec2 uv) {
	return normalize(hash12(uv*100.0)*6.28);
}

void main() {
	vec4 col = texture(sampler2D(OESSAO_texture, OESSAO_smp), uv);
	vec3 pcol = texture(sampler2D(OESSAO_ptexture, OESSAO_smp), uv).xyz;
	vec3 ncol = texture(sampler2D(OESSAO_ntexture, OESSAO_smp), uv).xyz;
    vec3 dcol = texture(sampler2D(OESSAO_dtexture, OESSAO_smp), uv).xyz;

	vec3 up = vec3(normalize(getRandom(uv)*getRandom(pcol.xy)),
                        normalize(getRandom(uv)*getRandom(pcol.xy)),
                        normalize(getRandom(uv)*getRandom(vec2(ncol.x,ncol.z))));
    //vec3 up = vec3(normalize(uv.x*pcol.z),normalize(uv.y*pcol.y),normalize(uv.x*pcol.x));
	vec3 t = normalize(cross(up,ncol));
	vec3 b = cross(ncol, t);
	mat3 TBN = mat3(t,b,ncol);

	float AO = 0.0;
	int i = 0;
	for(;i<KSIZE;i++) {
		vec3 samplePos = TBN*kernel[i].xyz;
		samplePos = pcol+samplePos*RADIUS;
		vec4 offs = proj*vec4(samplePos, 1.0);
		offs.xyz /= offs.w;
		offs.xyz = offs.xyz*0.5+0.5;
		float depth = texture(sampler2D(OESSAO_dtexture, OESSAO_smp), offs.xy).z;
		float rangeCheck = smoothstep(0.0,1.0,RADIUS/abs(pcol.z-depth));
		AO += (depth>=samplePos.z+BIAS?1.0:0.0)*(rangeCheck);
	}
	AO = 1.0-(AO/KSIZE);
	frag_color = vec4(AO)*col;
    //frag_color = vec4(AO);
}

@end

@program OESSAO vs_OESSAO fs_OESSAO
