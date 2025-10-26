/*Copyright (c) 2025, Tristan Wellman*/

#include <OE/OEUI.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

sg_pipeline_desc OEUIGetFontPipe(sg_shader shader, char *label) {
	char *_label = calloc(strlen(label)+1, sizeof(char));
	strcpy(_label, label);
	return (sg_pipeline_desc) {
		.shader = shader,
		.layout = {
			.attrs = {
				[OEUI_FATTRPOS].format = SG_VERTEXFORMAT_FLOAT2,
				[OEUI_FATTRTCOORD].format = SG_VERTEXFORMAT_FLOAT2,
			}
		},
		.primitive_type = SG_PRIMITIVETYPE_TRIANGLES,
		.index_type = SG_INDEXTYPE_NONE,
		.colors[0].pixel_format = SG_PIXELFORMAT_RGBA32F,
        .depth = {
			.compare = SG_COMPAREFUNC_ALWAYS,
			.write_enabled = false,
        },
		.label = _label
	};
}

OEUIFont *OEUILoadFont(char *filePath, char *ID) {
	if(!filePath) return NULL;
	OEUIFont *res = calloc(1, sizeof(OEUIFont));
	res->ID = calloc(strlen(ID)+1, sizeof(char));
	strcpy(res->ID, ID);
	res->glyph = calloc(OEUI_STBGLYPHSIZE, sizeof(stbtt_bakedchar));	
	res->fontSize = OEUI_DEFFONTSIZE;

	FILE *file = fopen(filePath, "rb");
	if(!file) {
		WLOG(WARN, "Failed to open .ttf file, skipping!");
		return NULL;
	}
	unsigned char *fb = calloc(1<<20, sizeof(unsigned char));
	fread(fb, 1, 1<<20, file);
	fclose(file);
	unsigned char *atlas = calloc(OEUI_ATLASSIZE, sizeof(unsigned char));
	stbtt_BakeFontBitmap(fb, 0, res->fontSize, 
			atlas, OEUI_ATLASWID, OEUI_ATLASWID, 
			OEUI_FSTCHR, OEUI_FENDCHR, (stbtt_bakedchar *)res->glyph);
	free(fb);
	res->atlasTex = sg_make_view(&(sg_view_desc){
			.texture.image = sg_make_image(&(sg_image_desc){
					.width = OEUI_ATLASWID, .height = OEUI_ATLASHEI,
					.pixel_format = SG_PIXELFORMAT_RGBA8,
					.data.mip_levels[0] = PTRRANGE(atlas, OEUI_ATLASSIZE*4),
					.label = ID
	})});
	return res;
}

void OEUIRenderText(OEUIFont *font, char *input) {

}
