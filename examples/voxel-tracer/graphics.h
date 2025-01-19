#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "engine/renderer.h"

#include <light.glsl.h>

typedef struct {
	sg_shader lightingShader;
	sg_pipeline windowPipe;
	Object screenQ;

	vs_params_t vs_params;
	voxel_data_t voxel_data;
	light_data_t light_data;
	sg_image curTex;
} GraphicsData;

void initScreenQ();
void gfxSetShaderParams(voxel_data_t voxel_data,
						light_data_t light_data,
						sg_image texture); 
void gfxApplyUniforms();

sg_shader *getLightShader();
sg_pipeline *getWindowPipe();
Object *getScreenQ();

#endif
