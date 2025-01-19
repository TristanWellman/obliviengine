#ifndef PLAYER_H
#define PLAYER_H

#include "engine/renderer.h"

typedef struct {
	vec3 pos;
	vec2 chunkPos;
	int id;
} Player;


void updatePlayer(Player *player);
void setPlayerPos(Player *player, vec3 pos);

#endif
