#ifndef LIGHT_H
#define LIGHT_H

#include "engine/renderer.h"
#include <light.glsl.h>

#define MAX_LIGHTS 300

typedef struct {
	vec3 position;
	vec3 color;
	int intensity;
	int id;
	char *ID;
} Light;

struct lightHandle {
	Light lights[MAX_LIGHTS];
	int totalLights;
	light_data_t uniform;
	int init;
};

void addLight(struct lightHandle *handle, 
		vec3 pos, vec3 color, int intensity, char *ID);
void setLightPos(struct lightHandle *handle, char *ID, vec3 newPos);
void updateLightUniform(struct lightHandle *handle); 
light_data_t getLightUniform(struct lightHandle *handle);

#endif
