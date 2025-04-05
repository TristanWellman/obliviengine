/*Copyright (c) 2025, Tristan Wellman
 *
 * Utility:
 * https://jacco.ompf2.com/2022/04/13/how-to-build-a-bvh-part-1-basics/
 * https://www.sci.utah.edu/~wald/Publications/2007/ParallelBVHBuild/fastbuild.pdf
 * https://www.johannes-guenther.net/BVHonGPU/BVHonGPU.pdf
 * */

#ifndef OEBVH_H
#define OEBVH_H

#include "util.h"
#include "OE.h"

typedef struct {
	vec3 min, max;
} AABB;

typedef struct {
	vec3 v1, v2, v3;
	vec3 centroid;
} OEBVHPrimitive;

/*Might be the one and only time you'll ever see me use a linked list.*/
typedef struct {
	AABB nodeBounds;
	OEBVHNode *left, *right;
	int isLeaf;
	OEBVHPrimitive *primitives;
} OEBVHNode;

/*Surface Area Heuristics (SAH)*/
float OEEvalSAH(OEBVHPrimitive *primitives, int s, int e);

#endif
