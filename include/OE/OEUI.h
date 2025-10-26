/*Copyright (c) 2025, Tristan Wellman*/

#ifndef OEUI_H
#define OEUI_H
#include <OE/OE.h>

#ifndef OEUI_DEFS
#	define OEUI_DEFS
#	define OEUI_STBGLYPHSIZE 95
#	define OEUI_DEFFONTSIZE 14
#	define OEUI_ATLASWID 512
#	define OEUI_ATLASHEI 512
#	define OEUI_ATLASSIZE \
		(OEUI_ATLASWID*OEUI_ATLASHEI)
#	define OEUI_FSTCHR 32
#	define OEUI_FENDCHR 96
#	define OEUI_FATTRPOS 0
#	define OEUI_FATTRTCOORD 1
#endif

/*This is a clone of stbtt_bakedchar*/
typedef struct {
	unsigned short x0,y0,x1,y1; /*coordinates of bbox in bitmap*/
	float xoff,yoff,xadvance;
} OEUI_bakedchar;

typedef struct {
	char *ID;
	OEUI_bakedchar *glyph;
	sg_view atlasTex;
	int fontSize :8;
} OEUIFont;

sg_pipeline_desc OEUIGetFontPipe(sg_shader shader, char *label);
OEUIFont *OEUILoadFont(char *filePath, char *ID);

#endif
