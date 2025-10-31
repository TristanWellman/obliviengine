/*
	Obliviengine FXAA quad shader.
	Based on: https://developer.download.nvidia.com/assets/gamedev/files/sdk/11/FXAA_WhitePaper.pdf
*/

@ctype vec2 vec2
@ctype vec3 vec3
@ctype vec4 vec4

@vs OEFXAA_vs

in vec2 OEFXAAQuad_position;
in vec2 OEFXAAQuad_texcoord;

out vec2 uv;

void main() {
	uv = OEFXAAQuad_texcoord;
	gl_Position = vec4(OEFXAAQuad_position, 0.0, 1.0);
}

@end

@fs OEFXAA_fs

layout(binding=0) uniform texture2D OEFXAA_texture;
layout(binding=0) uniform sampler OEFXAA_sampler;

layout(binding=1) uniform OEFXAA_resolution {
	vec2 resolution;
};

#define RESOLUTION vec2(1.0/resolution.x,1.0/resolution.y)
#define SPANMAX 8.0
#define REDUCEMUL (1.0/8.0)
#define REDUCEMIN (1.0/128.0)

/*https://stackoverflow.com/questions/596216/formula-to-determine-perceived-brightness-of-rgb-color*/
#define LUMWEIGH vec3(0.299,0.587,0.114)

in vec2 uv;
out vec4 frag_color;

void main() {
	vec2 texel = RESOLUTION;
	
	/*we get current & its neighbours*/
	vec3 rgbM = texture(sampler2D(OEFXAA_texture,OEFXAA_sampler), uv).rgb;
	vec3 rgbNW = texture(sampler2D(OEFXAA_texture,OEFXAA_sampler),uv+texel*vec2(-1.0,-1.0)).rgb;
	vec3 rgbNE = texture(sampler2D(OEFXAA_texture,OEFXAA_sampler),uv+texel*vec2(1.0,-1.0)).rgb;
	vec3 rgbSW = texture(sampler2D(OEFXAA_texture,OEFXAA_sampler),uv+texel*vec2(-1.0,1.0)).rgb;
	vec3 rgbSE = texture(sampler2D(OEFXAA_texture,OEFXAA_sampler),uv+texel*vec2(1.0,1.0)).rgb;

	/*covert to lumen*/
	float lumM = dot(rgbM, LUMWEIGH);
	float lumNW = dot(rgbNW, LUMWEIGH);
	float lumNE = dot(rgbNE, LUMWEIGH);
	float lumSW = dot(rgbSW, LUMWEIGH);
	float lumSE = dot(rgbSE, LUMWEIGH);

	float lumMin = min(lumM, min(min(lumNW,lumNE), min(lumSW,lumSE)));
	float lumMax = max(lumM, max(max(lumNW,lumNE), max(lumSW,lumSE)));

	/*directions*/
	vec2 dir = vec2(
		-((lumNW+lumNE)-(lumSW+lumSE)),
		((lumNW+lumSW)-(lumNE+lumSE)));
	
	float reduce = max((lumNW+lumNE+lumSW+lumSE)*(0.25*REDUCEMUL),REDUCEMIN);
	float rcpMin = 1.0/(min(abs(dir.x),abs(dir.y))+reduce);
	dir = clamp(dir*rcpMin,-SPANMAX,SPANMAX)*texel;

	/*edge blending*/
	vec3 rgbA = 0.5*(
		texture(sampler2D(OEFXAA_texture,OEFXAA_sampler),uv+dir*(1.0/3.0-0.5)).rgb+
		texture(sampler2D(OEFXAA_texture,OEFXAA_sampler),uv+dir*(2.0/3.0-0.5)).rgb);
	vec3 rgbB = 0.25*(
		texture(sampler2D(OEFXAA_texture,OEFXAA_sampler),uv+dir*-0.5).rgb+
		texture(sampler2D(OEFXAA_texture,OEFXAA_sampler),uv+dir*0.5).rgb)+0.5*rgbA;

	float lumB = dot(rgbB, LUMWEIGH);
	frag_color = vec4((lumB<lumMin||lumB>lumMax)?rgbA:rgbB,1.0);
}

@end

@program OEFXAA OEFXAA_vs OEFXAA_fs
