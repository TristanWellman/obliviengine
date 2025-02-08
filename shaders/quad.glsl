/*This is for the screen quad*/

@ctype mat4 mat4x4
@ctype vec2 vec2
@ctype vec3 vec3
@ctype vec4 vec4

@vs OEquad_vs

in vec2 OEquad_position;
in vec2 OEquad_texcoord;

out vec2 texcoord0;
out vec2 fragPos;

void main() {
	texcoord0 = OEquad_texcoord;
	fragPos = OEquad_position;
}

@end

@fs OEquad_fs

layout(binding=0) uniform texture2D OEquad_texture;
layout(binding=0) uniform sampler OEquad_smp;

in vec2 texcoord0;
in vec2 fragPos;

out vec4 frag_color;

void main() {
	vec3 texColor = texture(sampler2D(OEquad_texture, OEquad_smp), texcoord0).rgb;
	frag_color = vec4(texColor, 1.0);
}


@end

@program quad OEquad_vs OEquad_fs
