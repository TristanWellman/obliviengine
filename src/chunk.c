#include "chunk.h"

/*
 *typedef struct {
	vHandler *handle;
	vec3 positions[TILESIZE][TILESIZE];
	sg_image texture;
} Chunk;
 * */

void initChunk(Chunk *c, sg_image texture) {
	if(c==NULL) c = calloc(1, sizeof(Chunk));
	c->handle = calloc(1, sizeof(struct vHandler));
	initVHandle(c->handle, texture);
	int i,j;
	for(i=0;i<TILESIZE;i++) {
		for(j=0;j<TILESIZE;j++) {
			c->positions[i][j][0] = DEFC; 
			c->positions[i][j][1] = DEFC;
			c->positions[i][j][2] = DEFC;
		}
	}
	c->texture = texture;
}

void setFlatPlaneChunk(Chunk *c, vec3 initParams) {
	int i,j;
	int x=initParams[0],z=initParams[2];
	for(i=0;i<TILESIZE;i++) {
		for(j=0;j<TILESIZE;j++) {
			c->positions[i][j][0] = (x+i)*VOXSIZE;
			c->positions[i][j][1] = initParams[1];
			c->positions[i][j][2] = (z+j)*VOXSIZE;
		}
	}
}

void uploadChunkToVHandle(Chunk *c) {
	int i,j;
	for(i=0;i<TILESIZE;i++) {
		for(j=0;j<TILESIZE&&c->positions[i][j][0]!=DEFC;j++) {
			addVoxel(c->handle, c->positions[i][j], VOXSIZE);
		}
	}
	updateVoxelUniform(c->handle);
}
