#include "player.h"
#include "engine/renderer.h"
#include "chunk.h"

void updatePlayer(Player *player) {
	if(player==NULL) player = calloc(1, sizeof(Player));
	Camera *cam = getCamera();
	vec3_dup(player->pos, cam->ray_hit);
	player->pos[1] = 1.0f;
	player->chunkPos[0] = fabs(player->pos[0])/TILESIZE;
	player->chunkPos[1] = fabs(player->pos[2])/TILESIZE;
}

void setPlayerPos(Player *player, vec3 pos) {
	Camera *cam = getCamera();
	vec3_dup(player->pos, pos);
	vec3 offset = {-10.0f, 5.0f, -5.0f};
	vec3_add(cam->position, player->pos, offset);
	updateViewMat();
	player->chunkPos[0] = fabs(player->pos[0])/TILESIZE;
	player->chunkPos[1] = fabs(player->pos[2])/TILESIZE;
}
