/*This is for the screen quad*/

@ctype mat4 mat4x4
@ctype vec2 vec2
@ctype vec3 vec3
@ctype vec4 vec4

@vs OEquad_vs

in vec2 OEquad_position;
in vec2 OEquad_texcoord;

out vec2 texcoord0;

void main() {
	texcoord0 = OEquad_texcoord;
	gl_Position = vec4(OEquad_position, 0.0, 1.0);
}

@end

@fs OEquad_fs

layout(binding=0) uniform texture2D OEquad_texture;
layout(binding=0) uniform sampler OEquad_smp;

in vec2 texcoord0;

out vec4 frag_color;

void main() {
	/*vec3 texColor = texture(sampler2D(OEquad_texture, OEquad_smp), texcoord0).rgb;
	frag_color = vec4(texColor, 1.0);*/
	frag_color = texture(sampler2D(OEquad_texture, OEquad_smp), texcoord0);
}


@end

@program quad OEquad_vs OEquad_fs
