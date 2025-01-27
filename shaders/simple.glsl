/*
	This is the DEFAULT shader for ObliviEngine.
	Most default things for the engine require atleast these attributes in a shader.
*/

@ctype mat4 mat4x4
@ctype vec3 vec3
@ctype vec4 vec4

@vs vs
layout(binding=0) uniform vs_params {
    mat4 mvp;
    mat4 model;
};

in vec4 position;
in vec4 color0;
in vec3 normal0;
in vec2 texcoord0;

out vec4 color;
out vec3 normal;
out vec3 fragPos;
out vec2 texcoord;

void main() {
    gl_Position = mvp * position;
    color = color0;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    normal = normalize(normalMatrix * normal0);

    fragPos = vec3(model * position);
	texcoord = texcoord0;
}

@end

@fs fs

#define MAXLIGHTS 64

layout(binding=1) uniform light_params {
    vec4 positions[MAXLIGHTS]; /*[4] unused*/
    vec4 colors[MAXLIGHTS];
};

layout(binding=3) uniform fs_params {
    vec3 camPos;
};

layout(binding=3) uniform texture2D _texture;
layout(binding=3) uniform sampler smp;

in vec4 color;
in vec3 normal; 
in vec3 fragPos;
in vec2 texcoord;

out vec4 frag_color;

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

    for (int i = 0; i < MAXLIGHTS; i++) {
		if(colors[i]==vec4(0.0)) break;
        vec3 lightPos = vec3(positions[i]);
        vec3 lightColor = vec3(colors[i]);

        ambient += lightColor * materialAmbient;

        vec3 lightDir = normalize(lightPos - fragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        diffuse += diff * lightColor * materialDiffuse;

        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        specular += spec * lightColor * materialSpecular;
    }

    vec3 result = ambient + diffuse + specular;
    frag_color = vec4(result*vec3(color)*vec3(texcolor), color.a);

}

@end

@program simple vs fs
