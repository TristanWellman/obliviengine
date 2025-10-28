/*Font Shader*/

@ctype mat4 mat4x4
@ctype vec2 vec2
@ctype vec3 vec3
@ctype vec4 vec4

@vs OEFont_vs

layout(binding=0) uniform font_params {
	mat4 mvp;
};

in vec2 OEFont_position;
in vec2 OEFont_texcoord;

out vec2 texcoord0;

void main() {
	texcoord0 = OEFont_texcoord;
	gl_Position = mvp*vec4(OEFont_position, 0.0, 1.0);
}

@end

@fs OEFont_fs

/*TODO: just add the uniform binding for text colors to be passed from OEUIFont*/

layout(binding=0) uniform sampler OEFont_smp;
layout(binding=0) uniform texture2D OEFont_texture;

in vec2 texcoord0;

out vec4 frag_color;

void main() {
	float texColor = texture(sampler2D(OEFont_texture, OEFont_smp), texcoord0).a;
	frag_color = vec4(vec3(1.0), texColor);
}


@end

@program font OEFont_vs OEFont_fs
