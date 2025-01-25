/*Copyright (c) 2025 Tristan Wellman*/

#ifndef OELIGHTS_H
#define OELIGHTS_H

#include "renderer.h"
#include <simple.glsl.h> 

/*This needs to be the same in the shader*/
#define MAXLIGHTS 64

typedef struct {
	vec3 pos;
	Color color;
	char *ID;
} OELight;

struct LightData {
	OELight lights[MAXLIGHTS];
	int size; /*Current amount of lights*/
	light_params_t uniform;
};

void OEAddLight(char *ID, vec3 pos, Color color);
light_params_t getLightUniform();

#endif
