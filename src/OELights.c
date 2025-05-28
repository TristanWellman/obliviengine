/*Copyright (c) 2025 Tristan Wellman*/

#include <OE/OELights.h>

struct LightData *globalData;

void OEAddLight(char *ID, vec3 pos, Color color) {
	if(globalData==NULL) {
		globalData = calloc(1, sizeof(struct LightData));
		globalData->size = 0;
		int i;
		for(i=0;i<MAXLIGHTS;i++) {
			vec3_dup(globalData->lights[i].pos, (vec3){0.0f,0.0f,0.0f});
			globalData->lights[i].color = (Color){0.0f, 0.0f, 0.0f, 0.0f};
		}
	}

	int size = globalData->size;
	vec3_dup(globalData->lights[size].pos, pos);
	memcpy(&globalData->lights[size].color, &color, sizeof(color));
	globalData->lights[size].ID = calloc(strlen(ID)+1, sizeof(char));
	strcpy(globalData->lights[size].ID, ID);
	char buf[strlen(ID)+256];
	sprintf(buf, "Created light source[%d]: %s", globalData->size, ID);
	WLOG(INFO, buf);
	globalData->size++;
}

void OESetLightPosition(char *ID, vec3 pos) {
	if(globalData==NULL||ID==NULL) return;
	int i;
	for(i=0;i<globalData->size;i++) {
		if(globalData->lights[i].ID!=NULL) {
			if(!strcmp(ID, globalData->lights[i].ID)) vec3_dup(globalData->lights[i].pos, pos);
		}
	}
}

void OESetLightColor(char *ID, Color color) {
	if(globalData==NULL||ID==NULL) return;
	int i;
	for(i=0;i<globalData->size;i++) {
		if(globalData->lights[i].ID!=NULL) {
			if(!strcmp(ID, globalData->lights[i].ID)) globalData->lights[i].color = color;
		}
	}
}

light_params_t getLightUniform() {
	int i;
	for(i=0;i<globalData->size;i++) {
		globalData->uniform.positions[i][0] = globalData->lights[i].pos[0];
		globalData->uniform.positions[i][1] = globalData->lights[i].pos[1];
		globalData->uniform.positions[i][2] = globalData->lights[i].pos[2];
		globalData->uniform.positions[i][3] = 0.0f;
		globalData->uniform.colors[i][0] = globalData->lights[i].color.r;
		globalData->uniform.colors[i][1] = globalData->lights[i].color.g;
		globalData->uniform.colors[i][2] = globalData->lights[i].color.b;
		globalData->uniform.colors[i][3] = globalData->lights[i].color.a;
	}
	if(globalData==NULL) return (light_params_t){0};
	return globalData->uniform;
}

