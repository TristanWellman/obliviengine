/*Copyright (c) 2025, Tristan Wellman*/

#include <OE/OE.h>
#include <OE/OEUI.h>
#include <OE/font.glsl.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

/*private prototypes*/
sg_pipeline_desc OEUIGetFontPipe(sg_shader shader, char *label);
void OEUIApplyFontUniforms();

/*All these font rendering functions should be using the quad.glsl shader*/
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
		.index_type = SG_INDEXTYPE_UINT16,
		.colors[0].pixel_format = SG_PIXELFORMAT_RGBA32F,
		.colors[0].blend = (sg_blend_state){
			.enabled = true,
			.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
			.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
			.op_rgb = SG_BLENDOP_ADD,
			.src_factor_alpha = SG_BLENDFACTOR_ONE,
			.dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
			.op_alpha = SG_BLENDOP_ADD,
		},
        .depth = {
			.compare = SG_COMPAREFUNC_ALWAYS,
			.write_enabled = false,
        },
		.label = _label
	};
}

/*This is only gonna be called in OEInitRenderer*/
void OEUIInit(OEUIData *data, char *file) {
	if(!data) return;
	WASSERT(strstr(file, "OE.c"), "Invalid OEUIInit call!");
	data->defaultFont = NULL; /*I don't want to add a .ttf to the repo at this time*/
	data->prevText = NULL;
	sg_shader tmpFShade = sg_make_shader(font_shader_desc(sg_query_backend()));
	sg_pipeline_desc fpd = OEUIGetFontPipe(tmpFShade, "OEUIFontShade");
	sg_pipeline tmpFPipe = sg_make_pipeline(&fpd);
	data->fontShader.id = tmpFShade.id;
	data->fontPipeline.id = tmpFPipe.id;
	data->fontVbuf.id = 0;
	data->fontIbuf.id = 0;
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
	float *scaledAtlas = calloc(OEUI_ATLASSIZE*4, sizeof(float));
	stbtt_BakeFontBitmap(fb, 0, res->fontSize, 
			atlas, OEUI_ATLASWID, OEUI_ATLASWID, 
			OEUI_FSTCHR, OEUI_FENDCHR, (stbtt_bakedchar *)res->glyph);
	int i;
	for(i=0;i<OEUI_ATLASSIZE;i++) {
		scaledAtlas[i*4] = 1.0f;
		scaledAtlas[i*4+1] = 1.0f;
		scaledAtlas[i*4+2] = 1.0f;
		scaledAtlas[i*4+3] = (float)atlas[i]/255.0f;
	}
	sg_view atlasTmp = sg_make_view(&(sg_view_desc){
			.texture.image = sg_make_image(&(sg_image_desc){
					.width = OEUI_ATLASWID, .height = OEUI_ATLASHEI,
					.pixel_format = SG_PIXELFORMAT_RGBA32F,
					.data.mip_levels[0] = PTRRANGE(scaledAtlas, OEUI_ATLASSIZE*4),
					.label = ID
	})});
	res->atlasTex.id = atlasTmp.id;
	free(fb);
	free(atlas);
	free(scaledAtlas);
	return res;
}

void OEUIApplyFontUniforms() {
	font_params_t params;
	int w=0,h=0;
	OEGetWindowResolution(&w, &h);
	mat4x4_ortho(params.mvp, 0.0f, w, h, 0.0f, -1.0f, 1.0f);
	sg_apply_uniforms(UB_font_params, &SG_RANGE(params));
}

void OEUIRenderText(OEUIFont *font, char *input, int x, int y) {
	if(!font||!input) return;
	OEUIData *data = OEGetOEUIData();
	if(!data) {WLOG(WARN, "OEUI data is NULL!");return;}

	int len = strlen(input);
	int vSize = len*4*4, iSize = len*6;
	float *verts = calloc(vSize, sizeof(float));
	uint16_t *inds = calloc(iSize, sizeof(uint16_t));
	int i, v=0, ind=0;
	float posx=(float)x, posy=(float)y;
	for(i=0;i<len;i++,v+=16,ind+=6) {
		if(input[i]<OEUI_FSTCHR||input[i]>=OEUI_FENDCHR) continue;
		stbtt_aligned_quad q;
		stbtt_GetBakedQuad((stbtt_bakedchar *)font->glyph,
				OEUI_ATLASWID, OEUI_ATLASHEI,
				input[i]-OEUI_FSTCHR, &posx, &posy, &q, OEUI_STBGL);
		verts[v] = q.x0; verts[v+1] = q.y0;
		verts[v+2] = q.s0; verts[v+3] = q.t0;
		verts[v+4] = q.x1; verts[v+5] = q.y0;
		verts[v+6] = q.s1; verts[v+7] = q.t0;
		verts[v+8] = q.x1; verts[v+9] = q.y1;
		verts[v+10] = q.s1; verts[v+11] = q.t1;
		verts[v+12] = q.x0; verts[v+13] = q.y1;
		verts[v+14] = q.s0; verts[v+15] = q.t1;

		int b = i*4;
		inds[ind] = b;
		inds[ind+1] = b+2;
		inds[ind+2] = b+1;
		inds[ind+3] = b;
		inds[ind+4] = b+3;
		inds[ind+5] = b+2;
	}
	if(data->fontVbuf.id==SG_INVALID_ID) {
		sg_buffer tmp = sg_make_buffer(&(sg_buffer_desc){
				.usage.stream_update = true,
				.size = vSize*sizeof(float)+1,
				.label = "OEUIFontVBuf"
		});
		data->fontVbuf.id = tmp.id;
	}
	if(data->fontIbuf.id==SG_INVALID_ID) {
		sg_buffer tmp = sg_make_buffer(&(sg_buffer_desc){
				.usage.stream_update = true,
				.usage.index_buffer = true,
				.size = iSize*sizeof(uint16_t)+1,
				.label = "OEUIFontIBuf"
		});
		data->fontIbuf.id = tmp.id;
	}
	sg_buffer Vtmp = (sg_buffer){data->fontVbuf.id};
	sg_buffer Itmp = (sg_buffer){data->fontIbuf.id};
	sg_update_buffer(Vtmp, &PTRRANGE(verts, vSize));
	sg_update_buffer(Itmp, &PTRRANGE(inds, iSize));

	free(verts);
	free(inds);
	sg_view atlasTmp = (sg_view){font->atlasTex.id};
	sg_pipeline pipeTmp = (sg_pipeline){data->fontPipeline.id};
	sg_apply_pipeline(pipeTmp);
	sg_apply_bindings(&(sg_bindings){
			.vertex_buffers[0] = Vtmp,
			.index_buffer = Itmp,
			.views[0] = atlasTmp,
			.samplers[0] = OEGetSampler() 
	});
	OEUIApplyFontUniforms();
	sg_draw(0, ind, 1);

	if(data->prevText==NULL) data->prevText = calloc(len, sizeof(char));
	else data->prevText = (char *)realloc(data->prevText, sizeof(char)*len);
	strcpy(data->prevText, input);
}
