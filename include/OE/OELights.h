/*Copyright (c) 2025 Tristan Wellman*/

#ifndef OELIGHTS_H
#define OELIGHTS_H

#include "OE.h"
#include "simple.glsl.h"
#include "simple_low.glsl.h"

/*This needs to be the same in the shader*/
#define MAXLIGHTS 64

typedef struct {
	vec3 pos;
	Color color;
	char *ID;
} OELight;

struct LightData {
	int size; /*Current amount of lights*/
	OELight lights[MAXLIGHTS];
	light_params_t uniform;
};

void OEAddLight(char *ID, vec3 pos, Color color);
int OEDoesLightExist(char *ID);
void OERemoveLight(char *ID);
void OESetLightPosition(char *ID, vec3 pos);
void OESetLightColor(char *ID, Color color); 
light_params_t getLightUniform();
int getNumLights();

#endif
