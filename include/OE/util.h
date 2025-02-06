/*Copyright (c) 2024 Tristan Wellman*/

#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>

/*#define CVECTOR_LOGARITHMIC_GROWTH
#include <c-vector/cvector.h>*/

#define WLOG(logLevel, ...) \
	fprintf(stdout, "%s "#logLevel"  (%s:%d): %s\n", \
		__TIME__,__FILE__,__LINE__,__VA_ARGS__);

#define WASSERT(err, ...) \
	if(!(err)) {fprintf(stderr,__VA_ARGS__);exit(1);}	

#define RESETFCURSOR(file) \
	fseek(file, 0, SEEK_SET);

#define ARRLEN(x) \
		(sizeof(x)/sizeof(x[0]))

#ifndef PI
#define PI 3.14159f
#endif
#define DEG2RAD(_d) ((_d) * (PI / 180.0f))
#define RAD2DEG(_d) ((_d) * (180.0f / PI))

/*This is for cases where you cannot use a float[3] (vec3)*/
typedef struct {
	float x, y, z;
} Vec3;

typedef struct {
	float r,g,b,a;
} Color;

/*This is expecting a Color*/
#define RGBA255TORGBA1(src) \
	((__typeof(src)){(src.r)/255.0f, (src.g)/255.0f, \
	 (src.b)/255.0f, (src.a)/255.0f})

#define RGBA1TORGBA255(src) \
	((__typeof(src)){(src.r)*255.0f, (src.g)*255.0f, \
	 (src.b)*255.0f, (src.a)*255.0f})

/*
 *  keep in mind this is pretty sketchy since we declare a variable in a macro
 *  don't use it more than once per function bud.
 */
#define STARTAPPCHAR(str, c) \
	char *STRET = (char*)malloc(sizeof(char)*(strlen(str)+2)); \
	STRET[0] = c; 											   \
	memcpy(STRET+1, str, strlen(str));						   \
	STRET[strlen(str)+1] = '\0';							   \

/*Please don't ever use this piece of shit I made.
 * I am sorry.
 * */
#define EATTABS(line) 							\
	int i,j=0; 									\
	for(i=0;line[i];i++) {						\
		if(line[j+1]=='\0') break; 				\
		if(line[i]!='\t') line[j++] = line[i];} \
	line[j]='\0';

#endif
