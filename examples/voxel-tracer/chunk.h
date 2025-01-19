#ifndef CHUNK_H
#define CHUNK_H

#include "voxel.h"

#define TILESIZE 16
#define VOXSIZE 1
#define DEFC 1234567.8f

typedef struct {
	struct vHandler *handle;
	vec3 positions[TILESIZE][TILESIZE];
	sg_image texture;
} Chunk;

void initChunk(Chunk *c, sg_image texture);
void setFlatPlaneChunk(Chunk *c, vec3 initParams); 
void uploadChunkToVHandle(Chunk *c);

#endif
