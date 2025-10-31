/*Copyright (c) 2025, Tristan Wellman*/

#ifndef OEUI_H
#define OEUI_H
#include <OE/util.h>

/*sokol defs*/
typedef struct OEUI_view {uint32_t id;} OEUI_view;
typedef struct OEUI_buffer {uint32_t id;} OEUI_buffer;
typedef struct OEUI_shader {uint32_t id;} OEUI_shader;
typedef struct OEUI_pipeline {uint32_t id;} OEUI_pipeline;

#ifndef OEUI_DEFS
#	define OEUI_DEFS
#	define OEUI_DEFFONTSIZE 32
#	define OEUI_DEFFONTRESSCALE 40 /*window width / OEUI_DEFFONTSIZE*/
#	define OEUI_ATLASWID 1024
#	define OEUI_ATLASHEI 1024
#	define OEUI_ATLASSIZE \
		(OEUI_ATLASWID*OEUI_ATLASHEI)
#	define OEUI_FSTCHR 32
#	define OEUI_FENDCHR 127
#	define OEUI_STBGLYPHSIZE (OEUI_FENDCHR-OEUI_FSTCHR)
#	define OEUI_FATTRPOS 0
#	define OEUI_FATTRTCOORD 1
#	define OEUI_STBGL 1 /*stb_truetype.h: 1=opengl & d3d10+,0=d3d9*/
#	define OEUI_KEEPFBMEM (1<<0|1)
#endif

/*This is a clone of stbtt_packedchar*/
typedef struct {
	unsigned short x0,y0,x1,y1; /*coordinates of bbox in bitmap*/
	float xoff,yoff,xadvance;
	float xoff2,yoff2;
} OEUI_bakedchar;

typedef struct {
	char *ID, *path;
	unsigned char *fb; /*This is only kept if the user specifies OEUI_KEEPFBMEM*/ 
	OEUI_bakedchar *glyph;
	OEUI_view atlasTex;
	unsigned int fontSize :8;
} OEUIFont;

typedef struct {
	OEUIFont *defaultFont;
	OEUI_shader fontShader;
	OEUI_pipeline fontPipeline;
	OEUI_buffer fontVbuf, fontIbuf;
} OEUIData;

//sg_pipeline_desc OEUIGetFontPipe(sg_shader shader, char *label);
void OEUIInit(OEUIData *data, char *file);
void OEUIDestroyTTFBuffer(OEUIFont *font);
void OEUISetFontSize(OEUIFont *font, int size);
OEUIFont *OEUILoadFont(char *filePath, char *ID, int flag);
void OEUIRenderText(OEUIFont *font, char *input, int x, int y);

#endif
