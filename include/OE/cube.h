/*Copyright (c) 2025 Tristan Wellman
 *
 * NOTE: These are just for default objects, idealy you should use OEMesh.
 *
 * */

#ifndef CUBE_H
#define CUBE_H
#include <stdint.h>

static const float planeVertices[] = {
	-5.0f, 0.0f, -5.0f,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
	5.0f, 0.0f, -5.0f,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
	5.0f, 0.0f,  5.0f,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
	-5.0f, 0.0f,  5.0f,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f
};

static const uint16_t planeIndices[] = {
    0, 1, 2,
	0, 2, 3 
};

/*These are the quad verts for the offscreen pass,
 * You can also use it for whatever you need it*/
static const float quadVertices[] = {
	-1.0f, -1.0f, 0.0f, 0.0f, 
	 1.0f, -1.0f, 1.0f, 0.0f,  
	-1.0f,  1.0f, 0.0f, 1.0f, 

	-1.0f,  1.0f, 0.0f, 1.0f, 
	 1.0f, -1.0f, 1.0f, 0.0f, 
	 1.0f,  1.0f, 1.0f, 1.0f, 
};

/*Pos, Color, Norms.
 * You shouldn't make these, use OEMesh to do it for you.*/
static const float cubeVertices[] = {
	-1.0f, -1.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,    0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
	1.0f, -1.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,    0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
	1.0f,  1.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,    0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
	-1.0f,  1.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,    0.0f,  0.0f, -1.0f,   0.0f, 1.0f,

	-1.0f, -1.0f,  1.0f,   1.0f, 1.0f, 1.0f, 1.0f,    0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
	1.0f, -1.0f,  1.0f,   1.0f, 1.0f, 1.0f, 1.0f,    0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
	1.0f,  1.0f,  1.0f,   1.0f, 1.0f, 1.0f, 1.0f,    0.0f,  0.0f,  1.0f,   0.0f, 1.0f,
	-1.0f,  1.0f,  1.0f,   1.0f, 1.0f, 1.0f, 1.0f,    0.0f,  0.0f,  1.0f,   1.0f, 1.0f,

	-1.0f, -1.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,   -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
	-1.0f,  1.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,   -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
	-1.0f,  1.0f,  1.0f,   1.0f, 1.0f, 1.0f, 1.0f,   -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
	-1.0f, -1.0f,  1.0f,   1.0f, 1.0f, 1.0f, 1.0f,   -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,

	1.0f, -1.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,    1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
	1.0f,  1.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,    1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
	1.0f,  1.0f,  1.0f,   1.0f, 1.0f, 1.0f, 1.0f,    1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
	1.0f, -1.0f,  1.0f,   1.0f, 1.0f, 1.0f, 1.0f,    1.0f,  0.0f,  0.0f,   1.0f, 0.0f,

	-1.0f, -1.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,    0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
	-1.0f, -1.0f,  1.0f,   1.0f, 1.0f, 1.0f, 1.0f,    0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
	1.0f, -1.0f,  1.0f,   1.0f, 1.0f, 1.0f, 1.0f,    0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
	1.0f, -1.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,    0.0f, -1.0f,  0.0f,   1.0f, 1.0f,

	-1.0f,  1.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,    0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
	-1.0f,  1.0f,  1.0f,   1.0f, 1.0f, 1.0f, 1.0f,    0.0f,  1.0f,  0.0f,   0.0f, 0.0f,
	1.0f,  1.0f,  1.0f,   1.0f, 1.0f, 1.0f, 1.0f,    0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
	1.0f,  1.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,    0.0f,  1.0f,  0.0f,   1.0f, 1.0f
};

static const uint16_t cubeIndices[] = {
    0, 1, 2,  0, 2, 3,
    6, 5, 4,  7, 6, 4,
    8, 9, 10,  8, 10, 11,
    14, 13, 12,  15, 14, 12,
    16, 17, 18,  16, 18, 19,
    22, 21, 20,  23, 22, 20
};

#endif
