/* Copyright (c) 2025 MinervaWare LLC
 * simple box blur bloom shader.
 * */

@ctype vec2 vec2
@ctype vec3 vec3
@ctype vec4 vec4

@vs OEBQuad_vs

in vec2 OEBQuad_position;
in vec2 OEBQuad_texcoord;

out vec2 uv;

void main() {
	uv = OEBQuad_texcoord;
	gl_Position = vec4(OEBQuad_position,0,1);
}

@end

@fs OEBQuad_fs

layout(binding=0) uniform texture2D OEBTex;
layout(binding=0) uniform sampler OEBSmp;

layout(binding=1) uniform OEBloom_params {
	float thresh;
	float strength;
	vec2 resolution;
};
/*fxaa.glsl*/
#define LUMWEIGH vec3(0.299,0.587,0.114)

in vec2 uv;
out vec4 frag_color;

void main() {
	vec2 texel = vec2(1.0/resolution.x,1.0/resolution.y);
	vec4 col = texture(sampler2D(OEBTex,OEBSmp), uv);
	float lum = dot(col.rgb, LUMWEIGH);

	float inverseRange = 1.0/(1.0-thresh);
	float mask = clamp((lum-thresh)*inverseRange,0.0,1.0);

	/*sum accumulates the surrounding pixel colors so we can average them.*/
	vec3 sum = vec3(0.0);
	sum += col.rgb;
	sum += texture(sampler2D(OEBTex,OEBSmp),uv+texel*vec2(-1,-1)).rgb;
	sum += texture(sampler2D(OEBTex,OEBSmp),uv+texel*vec2(0,-1)).rgb;
	sum += texture(sampler2D(OEBTex,OEBSmp),uv+texel*vec2(1,-1)).rgb;
	sum += texture(sampler2D(OEBTex,OEBSmp),uv+texel*vec2(-1,1)).rgb;
	sum += texture(sampler2D(OEBTex,OEBSmp),uv+texel*vec2(1,0)).rgb;
	sum += texture(sampler2D(OEBTex,OEBSmp),uv+texel*vec2(-1,1)).rgb;
	sum += texture(sampler2D(OEBTex,OEBSmp),uv+texel*vec2(0,1)).rgb;
	sum += texture(sampler2D(OEBTex,OEBSmp),uv+texel*vec2(1,1)).rgb;

	vec3 blur = (sum/9.0)*mask*strength;
	frag_color = vec4(col.rgb+blur, col.a);
}

@end

@program OEBQuad OEBQuad_vs OEBQuad_fs
