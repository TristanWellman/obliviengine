#ifndef VOXEL_H
#define VOXEL_H
#include "engine/renderer.h"
#include <light.glsl.h>

#define MAX_VOXELS 300

typedef struct {
	vec3 position;
	int size;
	int id;
} Voxel;

struct vHandler {
	Voxel voxelData[MAX_VOXELS];
	int totalVoxels;
	voxel_data_t uniform;
	sg_image texture;
	int init;
	int id;
};

void initVHandle(struct vHandler *handle, sg_image texture); 
int addVoxel(struct vHandler *handle,
		vec3 pos, int size);
void setVoxelPos(struct vHandler *handle, int vID,
		vec3 newPos);
void updateVoxelUniform(struct vHandler *handle);
voxel_data_t getVoxelUniform(struct vHandler *handle);

#endif
