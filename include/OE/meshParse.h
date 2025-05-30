/*Copyright (c) 2025 Tristan Wellman
 *
 * This is for parsing .obj files as well as OpenFOAM mesh vert/face files
 *
 * */
#ifndef MESHPARSE_H
#define MESHPARSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "util.h"

#define MAXDATA 100000

#define MAXTIMESTAMPS 10000
#define MAXMAGDATA 100000

#define VSIZE 3 /*x,y,z*/
#define TEXSIZE 2 /*u,v*/
#define NORMSIZE 3 /*dx,dy,dz*/
#define ISIZE 4 /*v1,v2,v3,v4*/

typedef struct {
	uint16_t **data;
	int cap;
	int size;
	int total;
} DynArrI;

typedef struct {
	float **data;
	int cap;
	int size;
	int total;
} DynArrD;

typedef struct {
	DynArrD verts;
	DynArrD vertTex;
	DynArrD vertNorms;
	/*Btw this is only called indices because I am too lazy to change all code instances to faces
	 * This is a DynArrI of FACES... */
	DynArrI indices;
	DynArrI normInds;
	DynArrI texInds;
	FILE *f;
	char *label;
} OEMesh;

struct OEMagnitude {
	DynArrD	values;
	int timeStamp;
};

typedef struct {
	DynArrD verts;
	/*The faces need to be converted to indices!*/
	DynArrI indices;
	DynArrD faces;

	int *owner, *neighbour;
	int osize, nsize, ocap, ncap;
	/*array of magnitude data for the model
	 * - Used for colors*/
	struct OEMagnitude *magnitudeTS;
	int maxTS, sizeTS;
} OEFOAMMesh;

/*This sketchy void ptr expects a FILE ptr*/
typedef struct {
	void *file;
	void *meshptr;
} OEThreadArg;

/*
 * Get the magnitude values for a mesh at a specific timestamp. The timestamp should be in your "path"
 * */
void OEParseMagnitudeTimeStamp(char *path, int timeStamp, OEFOAMMesh *mesh);

/*
 * Get the OpenFOAM PolyMesh model for your renderer.
 * Data is stored in vertices (x,y,z)
 * */
void OEParseFOAMObj(char* path, OEFOAMMesh* mesh);

/*
 * Load a ".obj" model into vertices and faces.
 * */
void OEParseObj(char *name, char *file, OEMesh *mesh);

/*
 * Scale a model up or down by "s"
 * */
void scaleMesh(OEMesh *mesh, float s); 


#ifdef __cplusplus
}
#endif
#endif
