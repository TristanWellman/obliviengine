#include "voxel.h"

void initVHandle(struct vHandler *handle, sg_image texture) {
	if(handle==NULL) handle = calloc(1, sizeof(struct vHandler));
	if(handle->init!=123445) {
		handle->init = 123445;
		handle->totalVoxels = 0;
		int i;
		for(i=0;i<MAX_VOXELS;i++) handle->voxelData[i].id = 0;
		handle->texture = texture;
	}
}

int addVoxel(struct vHandler *handle,
		vec3 pos, int size) {
	int i;
	if(handle->init!=123445) {
		handle->init = 123445;
		handle->totalVoxels = 0;
		for(i=0;i<MAX_VOXELS;i++) handle->voxelData[i].id = 0;
	}
	i = handle->totalVoxels;
	handle->voxelData[i].id = i+1;
	vec3_dup(handle->voxelData[i].position, pos);
	handle->voxelData[i].size = size;
	handle->totalVoxels++;
	/*char buf[1024];
	sprintf(buf, "%d Created Voxel at (%f, %f, %f)", i,
			handle.voxelData[i].position[0],
			handle.voxelData[i].position[1],
			handle.voxelData[i].position[2]);
			WLOG(INFO, buf);*/
	return handle->voxelData[i].id;
	
}

void setVoxelPos(struct vHandler *handle, int vID,
		vec3 newPos) {
	Voxel *v = &handle->voxelData[vID-1];
	vec3_dup(v->position, newPos);
}

void updateVoxelUniform(struct vHandler *handle) {
	int i;
	for(i=0;i<handle->totalVoxels&&i<MAX_VOXELS;i++) {
		handle->uniform.voxel_info[i][0] = 
			handle->voxelData[i].position[0];
		handle->uniform.voxel_info[i][1] =
			handle->voxelData[i].position[1];
		handle->uniform.voxel_info[i][2] = 
			handle->voxelData[i].position[2];
		handle->uniform.voxel_info[i][3] = (float)handle->voxelData[i].size;
		/*char buf[1024];
			sprintf(buf, "%d Created Voxel at (%f, %f, %f, %f)", i,
					handle->uniform.voxel_info[i][0],
					handle->uniform.voxel_info[i][1],
					handle->uniform.voxel_info[i][2],
					handle->uniform.voxel_info[i][3] );
			WLOG(INFO, buf);*/
	}
	handle->uniform.num_voxels = handle->totalVoxels;
}

voxel_data_t getVoxelUniform(struct vHandler *handle) {
	return handle->uniform;
}

