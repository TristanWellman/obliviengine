/*Copyright (c) 2025, Tristan Wellman*/

#include <OE/OE.h>
#include <OE/OEUI.h>
#include <OE/cube.h>
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
	sg_shader tmpFShade = sg_make_shader(font_shader_desc(sg_query_backend()));
	sg_pipeline_desc fpd = OEUIGetFontPipe(tmpFShade, "OEUIFontShade");
	sg_pipeline tmpFPipe = sg_make_pipeline(&fpd);
	data->fontShader.id = tmpFShade.id;
	data->fontPipeline.id = tmpFPipe.id;
	data->fontVbuf.id = 0;
	data->fontIbuf.id = 0;
	sg_buffer imgTmp = sg_make_buffer(&(sg_buffer_desc) {
				.data = SG_RANGE(quadVertices),
				.label = "quadVerts"});
	data->imageBuf.id = imgTmp.id;
}

void OEUIDestroyTTFBuffer(OEUIFont *font) {
	if(font&&font->fb) free(font->fb);
}

void OEUISetFontSize(OEUIFont *font, int size) {
	if(!font) return;
	if(size>256) size = 256;
	if(size==font->fontSize) return;

	if(font->glyph) {
		free(font->glyph);
		font->glyph = calloc(OEUI_STBGLYPHSIZE, sizeof(stbtt_packedchar));
	}
	font->fontSize = size;

	if(!font->fb) {
		FILE *file = fopen(font->path, "rb");
		if(!file) {
			WLOG(WARN, "Failed to open .ttf file, skipping!");
			return;
		}
		font->fb = calloc(1<<20, sizeof(unsigned char));
		fread(font->fb, 1, 1<<20, file);
		fclose(file);
	}
	unsigned char *atlas = calloc(OEUI_ATLASSIZE, sizeof(unsigned char));
	float *scaledAtlas = calloc(OEUI_ATLASSIZE*4, sizeof(float));
	stbtt_pack_context pc;
	stbtt_pack_range pr;
	memset(&pr, 0, sizeof(pr));
	stbtt_PackBegin(&pc, atlas, OEUI_ATLASWID, OEUI_ATLASHEI, 0, 3, NULL);
	stbtt_PackSetOversampling(&pc, 3,3);
	pr.chardata_for_range = (stbtt_packedchar *)font->glyph;
	pr.font_size = font->fontSize;
	pr.first_unicode_codepoint_in_range = OEUI_FSTCHR;
	pr.num_chars = (OEUI_FENDCHR-OEUI_FSTCHR);
	stbtt_PackFontRanges(&pc, font->fb, 0, &pr, 1);
	stbtt_PackEnd(&pc);

	int i;
	for(i=0;i<OEUI_ATLASSIZE;i++) {
		scaledAtlas[i*4] = 1.0f;
		scaledAtlas[i*4+1] = 1.0f;
		scaledAtlas[i*4+2] = 1.0f;
		scaledAtlas[i*4+3] = (float)atlas[i]/255.0f;
	}
	sg_image img = sg_query_view_image((sg_view){font->atlasTex.id});
	sg_update_image(img, &(sg_image_data){
			.mip_levels[0]=PTRRANGE(scaledAtlas, OEUI_ATLASSIZE*4)});
	free(atlas);
	free(scaledAtlas);
}

OEUIFont *OEUILoadFont(char *filePath, char *ID, int flag) {
	if(!filePath) return NULL;
	OEUIFont *res = calloc(1, sizeof(OEUIFont));
	res->ID = calloc(strlen(ID)+1, sizeof(char));
	res->path = calloc(strlen(filePath)+1, sizeof(char));
	strcpy(res->ID, ID);
	strcpy(res->path, filePath);
	res->glyph = calloc(OEUI_STBGLYPHSIZE, sizeof(stbtt_packedchar));	
	res->fontSize = OEUI_DEFFONTSIZE;
	
	FILE *file = fopen(filePath, "rb");
	if(!file) {
		WLOG(WARN, "Failed to open .ttf file, skipping!");
		return NULL;
	}
	fseek(file, 0, SEEK_END);
	res->fbSize = ftell(file);
	rewind(file);
	res->fb = calloc(res->fbSize, sizeof(unsigned char));
	fread(res->fb, 1, res->fbSize, file);
	fclose(file);
	unsigned char *atlas = calloc(OEUI_ATLASSIZE, sizeof(unsigned char));
	float *scaledAtlas = calloc(OEUI_ATLASSIZE*4, sizeof(float));
	stbtt_pack_context pc;
	stbtt_pack_range pr;
	memset(&pr, 0, sizeof(pr));
	memset(&pc, 0, sizeof(pc));
	stbtt_PackBegin(&pc, atlas, OEUI_ATLASWID, OEUI_ATLASHEI, 0, 3, NULL);
	stbtt_PackSetOversampling(&pc, 3,3);
	pr.chardata_for_range = (stbtt_packedchar *)res->glyph;
	pr.font_size = res->fontSize;
	pr.first_unicode_codepoint_in_range = OEUI_FSTCHR;
	pr.num_chars = (OEUI_FENDCHR-OEUI_FSTCHR);
	stbtt_PackFontRanges(&pc, res->fb, 0, &pr, 1);
	stbtt_PackEnd(&pc);

	int i;
	for(i=0;i<OEUI_ATLASSIZE;i++) {
		scaledAtlas[i*4] = 1.0f;
		scaledAtlas[i*4+1] = 1.0f;
		scaledAtlas[i*4+2] = 1.0f;
		scaledAtlas[i*4+3] = (float)atlas[i]/255.0f;
	}
	sg_view atlasTmp = sg_make_view(&(sg_view_desc){
			.texture.image = sg_make_image(&(sg_image_desc){
					.usage.immutable = false,
					.usage.stream_update = true,
					.width = OEUI_ATLASWID, .height = OEUI_ATLASHEI,
					.pixel_format = SG_PIXELFORMAT_RGBA32F,
					.label = ID
	})});
	sg_image img = sg_query_view_image((sg_view){res->atlasTex.id});
	sg_update_image(img, &(sg_image_data){
			.mip_levels[0]=PTRRANGE(scaledAtlas, OEUI_ATLASSIZE*4)});
	res->atlasTex.id = atlasTmp.id;
	if(flag!=OEUI_KEEPFBMEM) {free(res->fb);res->fb=NULL;}
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
		stbtt_GetPackedQuad((stbtt_packedchar *)font->glyph,
				OEUI_ATLASWID, OEUI_ATLASHEI,
				(int)((unsigned char)input[i]-OEUI_FSTCHR), 
				&posx, &posy, &q, OEUI_STBGL);
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
	/*if(data->fontVbuf.id==SG_INVALID_ID) {
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
	}*/
	/* I know making and destroying buffers for each call
	 * is ineffecient, buf as of now that's what I need
	 * to dynamically update the buffers per call withought killing Sokol!*/
	sg_buffer vBuf = sg_make_buffer(&(sg_buffer_desc){
			.usage.stream_update = true,
			.size = vSize*sizeof(float)+1,
			.label = "OEUIFontVBuf"
	});
	sg_buffer iBuf = sg_make_buffer(&(sg_buffer_desc){
			.usage.stream_update = true,
			.usage.index_buffer = true,
			.size = iSize*sizeof(uint16_t)+1,
			.label = "OEUIFontIBuf"
	});
	sg_update_buffer(vBuf, &PTRRANGE(verts, vSize));
	sg_update_buffer(iBuf, &PTRRANGE(inds, iSize));

	free(verts);
	free(inds);
	sg_view atlasTmp = (sg_view){font->atlasTex.id};
	sg_pipeline pipeTmp = (sg_pipeline){data->fontPipeline.id};
	sg_apply_pipeline(pipeTmp);
	sg_apply_bindings(&(sg_bindings){
			.vertex_buffers[0] = vBuf,
			.index_buffer = iBuf,
			.views[0] = atlasTmp,
			.samplers[0] = OEGetSampler() 
	});
	OEUIApplyFontUniforms();
	sg_draw(0, ind, 1);

	sg_destroy_buffer(vBuf);
	sg_destroy_buffer(iBuf);
}

void OEUIDrawImage(OEUI_view image) {
	if(image.id==SG_INVALID_ID) {
		WLOG(WARN, "Invalid image view, skipping draw!");
		return;
	}
	OEUIData *data = OEGetOEUIData();
	sg_apply_pipeline(OEGetRTP());
	sg_apply_bindings(&(sg_bindings){
		.vertex_buffers[0] = (sg_buffer){data->imageBuf.id},
		.views[0] = (sg_view){image.id},
		.samplers[0] = OEGetSampler()
	});
	sg_draw(0,6,1);
}

void OEUIDrawImageEx(OEUI_view image, int x, int y, int w, int h) {
	if(image.id==SG_INVALID_ID) {
		WLOG(WARN, "Invalid image view, skipping draw!");
		return;
	}
	int sx=0, sy=0;
	OEGetWindowResolution(&sx, &sy);
	OEUIData *data = OEGetOEUIData();

	float x0 = ((float)x/(float)sx)*2.0f-1.0f;
	float y0 = 1.0f-((float)y/(float)sy)*2.0f;
	float x1 = (((float)x+(float)w)/(float)sx)*2.0f-1.0f;
	float y1 = 1.0f-(((float)y+(float)h)/(float)sy)*2.0f;
	float imgQuadVertices[] = {
		x0, y0, 0.0f, 0.0f, 
		x1, y0, 1.0f, 0.0f,  
		x0, y1, 0.0f, 1.0f, 
		x0, y1, 0.0f, 1.0f, 
		x1, y0, 1.0f, 0.0f, 
		x1, y1, 1.0f, 1.0f, 
	};

	sg_buffer imgBuf = sg_make_buffer(&(sg_buffer_desc) {
				.data = SG_RANGE(imgQuadVertices),
				.label = "imgQuadVerts"});

	sg_apply_pipeline(OEGetRTP());
	sg_apply_bindings(&(sg_bindings){
		.vertex_buffers[0] = imgBuf,
		.views[0] = (sg_view){image.id},
		.samplers[0] = OEGetSampler()
	});
	sg_draw(0,6,1);
	sg_destroy_buffer(imgBuf);
}


