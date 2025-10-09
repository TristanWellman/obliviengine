/*Copyright (c) 2025, Tristan Wellman*/

#ifndef TEXTURE_H
#define TEXTURE_H

#include "OE.h"

#define MAX_TEX 10000

typedef struct {
	sg_view tex;
	char *ID;
} Texture;

struct textureHandle {
	Texture *textures;
	int cap;
	int total;
};

void addTexture(char *ID, char *path);
sg_view getTexture(char *ID);

#endif

