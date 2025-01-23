/*Copyright (c) 2025 Tristan Wellman
 *
 * This is for parsing .obj files as well as OpenFOAM mesh vert/face files
 *
 * */
#ifndef MESHPARSE_H
#define MESHPARSE_H

#include "util.h"

#define MAXDATA 1000

#define VSIZE 3 /*x,y,z*/
#define TEXSIZE 2 /*u,v*/
#define NORMSIZE 3 /*dx,dy,dz*/

typedef struct {
	double **data;
	int cap;
	int size;
} DynArr;

typedef struct {
	DynArr verts;
	DynArr vertTex;
	DynArr vertNorms;
	DynArr indices;
	FILE *f;
	char *label;
} OEMesh;

void OEParseObj(char *file, OEMesh *mesh);

#endif
