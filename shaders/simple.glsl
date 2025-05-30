/*
	This is the DEFAULT shader for ObliviEngine.
	Most default things for the engine require atleast these attributes in a shader.
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


layout(binding=0) uniform vs_params {
    mat4 mvp;
    mat4 model;
};

layout(binding=1) uniform light_params {
    vec4 positions[MAXLIGHTS]; /*[4] unused*/
    vec4 colors[MAXLIGHTS];
};

layout(binding=3) uniform fs_params {
    vec3 camPos;
};

@end

@vs vs
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

@fs fs
@include_block uniforms

layout(binding=3) uniform texture2D _texture;
layout(binding=3) uniform sampler smp;

in vec4 color;
in vec3 normal; 
in vec3 fragPos;
in vec2 texcoord;

layout(location=0) out vec4 frag_color;
layout(location=1) out vec4 depth_color;
layout(location=2) out vec4 normal_color;
layout(location=3) out vec4 position_color;

void depth() {
	float depth = gl_FragCoord.z;
	depth_color = vec4(vec3(depth),1.0);
}

void normal_c() {
	normal_color = vec4(normal,1.0);
}

void position() {
	position_color = vec4(fragPos,1.0);
}

/*Phong Lighting for simple shader*/
void main() {
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(camPos - fragPos);

    vec3 ambient = vec3(0.0);
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);

    float shininess = 32.0;
    vec3 materialAmbient = vec3(0.1);
    vec3 materialDiffuse = vec3(0.7);
    vec3 materialSpecular = vec3(0.5);

	vec3 texcolor = texture(sampler2D(_texture, smp), texcoord).rgb;
	
	int al = 0;
	for(al=0;al<MAXLIGHTS&&(colors[al]==vec4(0.0));al++);
	float lightScale = 1.0/max(al,1);
	int i;
    for(i = 0; i < MAXLIGHTS; i++) {
		if(colors[i]==vec4(0.0)) break;
        vec3 lightPos = vec3(positions[i]);
        vec3 lightColor = vec3(colors[i]);

		float d = length(lightPos-fragPos);
		float atten = 1.0/(1.0+0.09*d+0.01*d*d);

        ambient += (lightScale * atten * lightColor * materialAmbient)*2.0;

        vec3 lightDir = normalize(lightPos - fragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        diffuse += (lightScale * atten * diff * lightColor * materialDiffuse);

        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        specular += (lightScale * atten * spec * lightColor * materialSpecular);
    }

    vec3 result = ambient + diffuse + specular;
    frag_color = vec4(result*vec3(color)*vec3(texcolor), 1.0);

	depth();
	normal_c();
	position();

}

@end

@program simple vs fs
