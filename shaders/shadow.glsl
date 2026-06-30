/* Copyright (c) 2026 Tristan Wellman*/

@ctype mat4 mat4x4
@ctype vec3 vec3
@ctype vec4 vec4
@ctype vec2 vec2

@vs vs_shadow

layout(binding=0) uniform shadow_vs_params {
    mat4 model;
	mat4 view;
	mat4 proj;
};

in vec3 position;
in vec4 color;
in vec3 normal0;
in vec2 texcoord0;

void main() {
	gl_Position = color*vec4(normal0,1.0)*vec4(texcoord0,1.0,1.0);
    gl_Position = ((proj*view)*model)*vec4(position, 1.0);
#ifdef _OE_VULKAN
    gl_Position.y *= -1.0;
    gl_Position.z = (gl_Position.z+gl_Position.w)*0.5;
#endif
}

@end

@fs fs_shadow

layout(location=0) out vec4 frag_color;
layout(location=1) out vec4 depth_color;
layout(location=2) out vec4 normal_color;
layout(location=3) out vec4 position_color;

void main() {
    frag_color = vec4(vec3(gl_FragCoord.z),1.0);
}

@end

@program shadow vs_shadow fs_shadow
