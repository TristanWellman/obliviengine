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

#define RAYS 32
#define STEPS 8
#define RADIUS 0.5
#define BIAS 0.05
#define INTENSITY 2.0
#define PI 3.1415926
#define SCALE 43758.5453

layout(binding=4) uniform OESSGI_params {
	mat4 proj;
};

layout(binding=0) uniform sampler OESSGI_smp;
layout(binding=0) uniform texture2D OESSGI_texture;
layout(binding=2) uniform texture2D OESSGI_ntexture;
layout(binding=3) uniform texture2D OESSGI_ptexture;

in vec2 uv;
out vec4 frag_color;

/*https://www.shadertoy.com/view/Ms33WB*/
#define MOD3 vec3(.1031,.11369,.13787)
float hash12(vec2 p) {
	vec3 p3  = fract(vec3(p.xyx) * MOD3);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}
/*float getRandom(vec2 uv) {
	return normalize(hash12(uv*100.0)*6.28);
}*/
float getRandom(vec2 uv) {
	return fract(sin(dot(uv,vec2(12.98,78.23)))*SCALE);
}

void main() {
	vec3 col = texture(sampler2D(OESSGI_texture, OESSGI_smp), uv).rgb;
	vec3 pcol = texture(sampler2D(OESSGI_ptexture, OESSGI_smp), uv).xyz;
	if(length(pcol)<0.001) {frag_color=vec4(col,1.0);return;}
	vec3 ncol = normalize(texture(sampler2D(OESSGI_ntexture, OESSGI_smp), uv).xyz*2.0-1.0);

	vec3 up = abs(ncol.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 t = normalize(cross(up,ncol));
	vec3 b = cross(ncol, t);
	mat3 TBN = mat3(t,b,ncol);
	vec3 GI = vec3(0.0);
	float seed = getRandom(uv*pcol.z);

	int i=0, hits=0;
	for(;i<RAYS;i++) {
		float phi = (PI*2)*getRandom(vec2(i,seed));
		float cosT = getRandom(vec2(seed,i));
		float sinT = sqrt(max(0.0,1.0-cosT*cosT));
		vec3 dirVS = vec3(cos(phi)*sinT, sin(phi)*sinT, cosT);
		vec3 rayDir = normalize(TBN*dirVS);
		int j = 0;
		for(;j<=STEPS;j++) {
			float ta = (float(j)/float(STEPS))*RADIUS;
			vec3 samplePos = pcol+rayDir*ta;
			vec4 clip = proj*vec4(samplePos,1.0);
			vec2 suv = (clip.xy/clip.w)*0.5+0.5;
			if(suv.x<0.0||suv.x>1.0||suv.y<0.0||suv.y>1.0) continue;
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
	float AO = 1.0-((float(hits)/float(RAYS))*1.5);
	AO = clamp(AO,0.2,1.0);
	frag_color = vec4((col+GI),1.0);
}

@end

@program OESSGI vs_OESSGI fs_OESSGI

