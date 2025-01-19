#ifndef WORLD_H
#define WORLD_H

#include "voxel.h"
#include "light.h"
#include "graphics.h"
#include "chunk.h"
#include "player.h"

#define WORLDSIZE 128

typedef struct {
	Chunk chunks[WORLDSIZE][WORLDSIZE];
	Player player;
} World;

void initWorld();
void drawWorld();

#endif
