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

/*This is for cases where you cannot use a float[3] (vec3)*/
typedef struct {
	float x, y, z;
} Vec3;
typedef struct {
	float x, y;
} Vec2;

typedef struct {
	float r, g, b, a;
} Color;

#define WLOG(_logLevel, ...) \
	fprintf(stdout, "%s "#_logLevel"  (%s:%d): %s\n", \
		__TIME__,__FILE__,__LINE__,__VA_ARGS__);

#define WASSERT(_err, ...) \
	if(!(_err)) {fprintf(stderr,__VA_ARGS__);exit(1);}	

#define RESETFCURSOR(_file) \
	fseek(_file, 0, SEEK_SET);

#define ARRLEN(_x) \
		(sizeof(_x)/sizeof(_x[0]))

#ifndef PI
#define PI 3.14159f
#endif
#define DEG2RAD(_d) ((_d) * (PI / 180.0f))
#define RAD2DEG(_d) ((_d) * (180.0f / PI))

/*Xor Swap*/
#define WSWAP(_x, _y) \
	((_x)^=(_y));((_y)^=(_x));((_x)^=(_y));

/*
 * My implementation of everyone's favorite
 * Fast Inverse Square Root Algorithm
 * but as C macros.
 * 
 * We use 'uint32_t' instead of 'long' because MSVC longs are 32-bit
 * whereas GCC/Clang is 64-bit which messes it up.
 * */
#define QSMAGIC 0x5f3759df
#define QISQRT(_x) \
	({float y=(_x);uint32_t i=*(uint32_t*)&y; \
	  i=(QSMAGIC-(i>>1));y=*(float*)&i; \
	  y*=(1.5f-(0.5f*(_x)*y*y));y;})

/*Normalization macro
 * takes in a Vec3 */
#define WNORM(_x) ((__typeof__(_x)) { \
	(_x.x)*QISQRT((_x.x*_x.x)+(_x.y*_x.y)+(_x.z*_x.z)), \
	(_x.y)*QISQRT((_x.x*_x.x)+(_x.y*_x.y)+(_x.z*_x.z)), \
	(_x.z)*QISQRT((_x.x*_x.x)+(_x.y*_x.y)+(_x.z*_x.z))})

/*_v1: src, _v2: dest*/
#define VEC3TOVEC3F(_v1, _v2) \
	(_v2)[0]=(_v1.x);(_v2)[1]=(_v1.y);(_v2)[2]=(_v1.z); 
 /*
  * This is a linear approximation macro
  * Very useless unless you specifically need NEAR values and are fine with the extra cost.
  */
#define APPRPRECISION 0.01
#define DIRAPPROX(_f, _a) \
	(((_f)((_a)+APPRPRECISION)-(_f)((_a)-APPRPRECISION))/(APPRPRECISION*2.0f))
#define APPROX(_f, _a, _x) \
	((_f)(_a)+DIRAPPROX(_a)*((_x)-(_a)))

/*This is expecting a Color*/
#define RGBA255TORGBA1(_src) \
	((__typeof(_src)){(_src.r)/255.0f, (_src.g)/255.0f, \
	 (_src.b)/255.0f, (_src.a)/255.0f})

#define RGBA1TORGBA255(_src) \
	((__typeof(_src)){(_src.r)*255.0f, (_src.g)*255.0f, \
	 (_src.b)*255.0f, (_src.a)*255.0f})

/*Bitset structure declaration macro
 * _name - name of the struct
 * _s - number of bits
 *
 * #include <stdio.h>
 * 
 * WBITSETINIT(Foo, 100);
 *
 * int main() {
 * 		Foo bs;
 * 		FooReset(&bs, 0);
 * 		FooSet(&bs, 24);
 * 		if(FooGet(&bs, 24)) 
 * }
 *
 * */
/*#define WBITSETINIT(_name, _s) \
	static const _name##Bits=(_s); \
	static const _name##Bytes=((_s)+7)/8; \
	static const _name##Extra=(8*_name##Bytes)-(_s); \
	typedef struct {uint32_t _b[_name##Bytes];} _name; \
	static void _name##Set(_name *bs, unsigned i) { \
		if(i<_name##Bits) bs->_b[i>>3]|=(1u<<(i&7));} \
	static void _name##Reset(_name *bs, int value) { \
		memset(bs->b, \
	} \
*/

/*
 *  keep in mind this is pretty sketchy since we declare a variable in a macro
 *  don't use it more than once per function bud.
 */
#define STARTAPPCHAR(str, c) \
	char *STRET = (char*)malloc(sizeof(char)*(strlen(str)+2)); \
	STRET[0] = c; 											  \
	memcpy(STRET+1, str, strlen(str));						   \
	STRET[strlen(str)+1] = '\0';							   \


#endif

