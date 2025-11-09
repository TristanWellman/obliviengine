/*
	THIS IS NOT FINISHED

	Ray Tracing Default Shader for ObliviEngine.

	I would like to note shdc does not have a way to ignore duplicate usage of uniforms,
	I want to use the same fs and vs uniforms for multiple shaders but it puts the
	definition into the header no matter what :/
*/

@ctype mat4 mat4x4
@ctype vec3 vec3
@ctype vec4 vec4
@ctype vec2 vec2

@block uniforms

#define MAXLIGHTS 64
#define MAXSTEPS 100
#define MAXDIST 100.0
#define EPSILON 0.001
#define PI 3.14159265359

layout(binding=0) uniform rtvs_params {
    mat4 mvp;
    mat4 model;
};

layout(binding=1) uniform rtlight_params {
    vec4 positions[MAXLIGHTS]; /*[4] unused*/
    vec4 colors[MAXLIGHTS];
};

layout(binding=3) uniform rtfs_params {
    vec3 camPos;
};

@end
@vs vs_OERayTracer
@include_block uniforms

in vec3 position;
in vec4 color0;
in vec3 normal0;
in vec2 texcoord0;

out vec4 color;
out vec3 normal;
out vec3 fragPos;
out vec2 texcoord;

void main() {
    gl_Position = mvp * vec4(position,1.0);
    color = color0;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    normal = normalize(normalMatrix * normal0);

    fragPos = vec3(model * vec4(position,1.0));
	texcoord = texcoord0;
}

@end

@fs fs_OERayTracer
@include_block uniforms

layout(binding=3) uniform texture2D _texture;
layout(binding=3) uniform sampler smp;

in vec4 color;
in vec3 normal; 
in vec3 fragPos;
in vec2 texcoord;

out vec4 frag_color;

void main() {
	vec3 texcolor = texture(sampler2D(_texture, smp), texcoord).rgb;
	// these are just so the uniforms get generated while I work on this
	vec4 tmp = positions[0];
	vec4 tmp2 = colors[0];
	vec3 tmp3 = camPos*tmp2.rgb*tmp.rgb;
	frag_color = vec4(texcolor*tmp3, 1.0); 
}

@end

@program OERayTracer vs_OERayTracer fs_OERayTracer

