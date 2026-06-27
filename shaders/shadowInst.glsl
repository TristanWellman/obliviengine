/* Copyright (c) 2026 Tristan Wellman*/

@ctype mat4 mat4x4
@ctype vec3 vec3
@ctype vec4 vec4
@ctype vec2 vec2

@vs vs_shadowInst

layout(binding=0) uniform shadowInst_vs_params {
    mat4 model;
	mat4 view;
	mat4 proj;
};

in vec3 position;
in vec4 color0;
in vec3 normal0;
in vec2 texcoord0;
in vec4 instModelr0;
in vec4 instModelr1;
in vec4 instModelr2;
in vec4 instModelr3;

void main() {
	mat4 instModel = mat4(instModelr0, instModelr1, instModelr2, instModelr3);
	vec4 pos = instModel*vec4(position, 1.0);
    gl_Position = (proj*view)*pos;
}
@end

@fs fs_shadowInst

layout(location=0) out vec4 frag_color;
layout(location=1) out vec4 depth_color;
layout(location=2) out vec4 normal_color;
layout(location=3) out vec4 position_color;

void main() {
    frag_color = vec4(vec3(gl_FragCoord.z),1.0);
}

@end

@program shadowInst vs_shadowInst fs_shadowInst
