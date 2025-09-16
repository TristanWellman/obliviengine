/* Copyright (c) 2025 Tristan Wellman
 * This is the DEFAULT shader for ObliviEngine.
 * Most default things for the engine require atleast these attributes in a shader.
 * 08/30/35 - Changed phong lighting to mix in some pbr functionality.
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
	mat4 view;
};

layout(binding=1) uniform light_params {
    vec4 positions[MAXLIGHTS]; /*[4] unused*/
    vec4 colors[MAXLIGHTS]; /*[4] is intensity*/
};

layout(binding=3) uniform fs_params {
    vec3 camPos;
	int numLights;
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
out vec3 viewSpacePos;
out vec3 viewSpaceNorm;
out vec2 texcoord;

void main() {
    gl_Position = mvp * vec4(position,1.0);
    color = color0;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    normal = normalize(normalMatrix * normal0);

    fragPos = vec3(model * vec4(position,1.0));
	viewSpacePos = vec3(view*vec4(fragPos,1.0));
	viewSpaceNorm = normalize(mat3(view) * normal);
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
in vec3 viewSpacePos;
in vec3 viewSpaceNorm;
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
	normal_color = vec4(viewSpaceNorm*0.5+0.5,1.0);
}

void position() {
	position_color = vec4(viewSpacePos,1.0);
}

/*Phong Lighting for simple shader*/
void main() {
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(camPos - fragPos);

    vec3 ambient;

    float shininess = 32.0;
    vec3 materialAmbient = vec3(0.1);
    vec3 materialDiffuse = vec3(0.7);
    vec3 materialSpecular = vec3(0.5);

	ambient = materialAmbient*vec3(0.02);
	/* accumulate diffuse & specular */
	vec3 ad = vec3(0.0);
	vec3 as = vec3(0.0);

	vec3 texcolor = texture(sampler2D(_texture, smp), texcoord).rgb;
	
	int activeLight = clamp(numLights,0,MAXLIGHTS);
	int i;
    for(i=0;i<activeLight;i++) {
        vec3 lightPos = vec3(positions[i]);
		

		float range = positions[i].w;
		if(range<=0.001) range = 10.0;

        vec3 lightColor = colors[i].rgb;
		float intensity = colors[i].a*1.5;
		if(intensity<=0.0) intensity = 1.0;
		lightColor *= intensity;

		float d = length(lightPos-fragPos);
		float invDistSq = 1.0/max(d*d,0.01);
		float rf = clamp(1.0-(d/range),0.0,1.0);
		float sr = pow(rf,2.0);
		float atten = invDistSq*sr;

        vec3 lightDir = normalize(lightPos - fragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = lightColor*materialDiffuse*diff*atten;

        float spec = pow(max(dot(norm, normalize(lightDir+viewDir)), 0.0), shininess);
        vec3 specular = lightColor*materialSpecular*spec*atten*0.8;;
		
		ad += diffuse;
		as += specular;
    }
	
	vec3 hdr = ambient+ad*texcolor+as;
	vec3 mapped = hdr/(vec3(1.0)+hdr);

    vec3 result = pow(mapped, vec3(1.0/2.2));
    frag_color = vec4(result*vec3(color), 1.0);

	depth();
	normal_c();
	position();
}

@end

@program simple vs fs
