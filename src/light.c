#include "light.h"

void addLight(struct lightHandle *handle, 
		vec3 pos, vec3 color, int intensity, char *ID) {
	int i;
	if(handle->init!=123445) {
		handle->init = 123445;
		handle->totalLights = 0;
		for(i=0;i<MAX_LIGHTS;i++) {
			handle->lights[i].id = 0;
			handle->lights[i].intensity = 0;
		}
	}
	i = handle->totalLights;
	handle->lights[i].id = i+1;
	vec3_dup(handle->lights[i].position, pos);
	vec3_dup(handle->lights[i].color, color);
	handle->lights[i].intensity = intensity;
	if(ID!=NULL) {
		handle->lights[i].ID = calloc(strlen(ID)+1, sizeof(char));
		strcpy(handle->lights[i].ID, ID);
	}
	handle->totalLights++;
}

void setLightPos(struct lightHandle *handle, char *ID, vec3 newPos) {
	int i;
	for(i=0;i<handle->totalLights;i++) {
		if(handle->lights[i].ID!=NULL&&
				!strcmp(handle->lights[i].ID, ID)) {
			vec3_dup(handle->lights[i].position, newPos);
			break;
		}
	}
}

void updateLightUniform(struct lightHandle *handle) {
	int i;
	for(i=0;i<handle->totalLights;i++) {
		handle->uniform.light_positions[i][0] = 
			handle->lights[i].position[0];
		handle->uniform.light_positions[i][1] = 
			handle->lights[i].position[1];
		handle->uniform.light_positions[i][2] = 
			handle->lights[i].position[2];
		handle->uniform.light_positions[i][3] = 
			(float)handle->lights[i].intensity;

		handle->uniform.light_colors[i][0] = 
			handle->lights[i].color[0];
		handle->uniform.light_colors[i][1] = 
			handle->lights[i].color[1];
		handle->uniform.light_colors[i][2] = 
			handle->lights[i].color[2];
		handle->uniform.light_colors[i][3] = 1.0f;
	
		handle->uniform.num_lights = handle->totalLights;
	}
}

light_data_t getLightUniform(struct lightHandle *handle) {
	return handle->uniform;
}

