/*Copyright (c) 2025 Tristan Wellman*/

#include <OE/OELights.h>

struct LightData *globalData = NULL;

void OEAddLight(char *ID, vec3 pos, Color color) {
	if(globalData==NULL) {
		globalData = calloc(1, sizeof(struct LightData));
		globalData->size = 0;
		int i;
		for(i=0;i<MAXLIGHTS;i++) {
			vec3_dup(globalData->lights[i].pos, (vec3){0.0f,0.0f,0.0f});
			globalData->lights[i].color = (Color){0.0f, 0.0f, 0.0f, 0.0f};
			globalData->lights[i].ID = NULL;
		}
	}
	if(OEDoesLightExist(ID)) return;
	int size = globalData->size;
	if(size>=MAXLIGHTS) {
		WLOG(WARN, "Too many lights, not adding.");
		return;
	}
	vec3_dup(globalData->lights[size].pos, pos);
	memcpy(&globalData->lights[size].color, &color, sizeof(color));
	globalData->lights[size].ID = calloc(strlen(ID)+1, sizeof(char));
	strcpy(globalData->lights[size].ID, ID);
	char buf[strlen(ID)+256];
	sprintf(buf, "Created light source[%d]: %s", globalData->size, ID);
	WLOG(INFO, buf);
	globalData->size++;
}

int OEDoesLightExist(char *ID) {
	if(globalData==NULL) return 0;
	int i, size = globalData->size;
	for(i=0;i<size&&globalData->lights[i].ID;i++) 
		if(!strcmp(globalData->lights[i].ID, ID)) return 1;
	return 0;
}

void OERemoveLight(char *ID) {
	/*I haven't done qsort with the lights yet, so this will be slow linear til' I fix that.*/
	int i, j;
	for(i=0;(i<globalData->size)&&(globalData->lights[i].ID!=NULL)
			&&(strcmp(ID, globalData->lights[i].ID)!=0);i++);
	if(i>=globalData->size||globalData->lights[i].ID==NULL) return;
	free(globalData->lights[i].ID);
	globalData->lights[i].ID = NULL;
	if(i<globalData->size-1) {
		memmove(&globalData->lights[i], &globalData->lights[i+1],
				(globalData->size-i-1)*sizeof(globalData->lights[0]));
		/*for(;i<globalData->size-1;i++) {
			vec3_dup(globalData->lights[i].pos, globalData->lights[i+1].pos);
			globalData->lights[i].color = globalData->lights[i+1].color;
			if(globalData->lights[i].ID==NULL&&globalData->lights[i+1].ID!=NULL) 
				globalData->lights[i].ID = calloc(strlen(globalData->lights[i+1].ID)+1, sizeof(char));
			strcpy(globalData->lights[i].ID, globalData->lights[i+1].ID);
		}*/
	}
	char buf[strlen(ID)+256];
	sprintf(buf, "Removed light source[%d]: %s", globalData->size, ID);
	WLOG(INFO, buf);

	globalData->size--;
	globalData->lights[globalData->size].ID = NULL;
	vec3_dup(globalData->lights[globalData->size].pos, (vec3){0.0f,0.0f,0.0f});
	globalData->lights[globalData->size].color = (Color){0.0f,0.0f,0.0f,0.0f};
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
	if(globalData==NULL) return (light_params_t){0};
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
	return globalData->uniform;
}

int getNumLights() {
	if(globalData==NULL) return 0;
	return globalData->size;
}



