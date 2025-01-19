#include "world.h"
#include "voxel.h"
#include "light.h"
#include "graphics.h"
#include "chunk.h"

World *world;
struct lightHandle *lHandle;

void loadAssets() {
	addTexture("assets/stone.png", "stone");
}

void initWorld() {
	world = calloc(1, sizeof(World));
	loadAssets();	
	lHandle = calloc(1, sizeof(struct lightHandle));
	
	/*chunks*/
	int i,j;
	for(i=0;i<WORLDSIZE;i++) {
		for(j=0;j<WORLDSIZE;j++) {
			initChunk(&world->chunks[i][j], getTexture("stone"));
			setFlatPlaneChunk(&world->chunks[i][j],
					(vec3){(float)(i*TILESIZE), 0.0f, (float)(j*TILESIZE)});
			world->player.id = addVoxel(
				world->chunks[i][j].handle, world->player.pos, 1);
			uploadChunkToVHandle(&world->chunks[i][j]);
		}
	}

	setPlayerPos(&world->player, (vec3){WORLDSIZE/2, 1, WORLDSIZE/2});

	/*light*/
	vec3 pos;
	vec3_dup(pos, world->player.pos);
	addLight(lHandle, (vec3){pos[0]-10, pos[1]+2, pos[2]}, 
			(vec3){1.0f, 0.82f, 0.63f}, 3.0f, "test");


}

void drawWorld() {
	updatePlayer(&world->player);
	Vec3 camPos = getCamPos();
	vec3 pos = {camPos.x, camPos.y, camPos.z};
	//setLightPos(lHandle, "test", (vec3){pos[0]-10, pos[1], pos[2]});
	int i = world->player.chunkPos[0];
	int j = world->player.chunkPos[1];
	/*int di, dj;
	for (dj = 1; dj >= -1; dj--) {          
    	for (di = 1; di >= -1; di--) {      
        	int ci = i + di;                
        	int cj = j + dj;                
        	if (ci >= 0 && ci < WORLDSIZE && cj >= 0 && cj < WORLDSIZE) {*/
            
				setVoxelPos(world->chunks[i][j].handle, world->player.id, world->player.pos);
            	updateVoxelUniform(world->chunks[i][j].handle);

            	sg_image curTex = world->chunks[i][j].texture;
            	updateLightUniform(lHandle);
            	gfxSetShaderParams(getVoxelUniform(world->chunks[i][j].handle),
            	        lHandle->uniform, curTex);
            	drawObjectTexEx(getScreenQ(), IMG_v_texture,
            	        curTex, (UNILOADER)gfxApplyUniforms);
        /*	}
    	}
	}*/
}
