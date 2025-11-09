/*Copyright (c) 2025, Tristan Wellman*/

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <OE/texture.h>

struct textureHandle handle;

void addTexture(char *ID, char *path, int fv) {
	if(handle.textures==NULL) {
		handle.cap = MAX_TEX;
		handle.total = 0;
		handle.textures = calloc(handle.cap, sizeof(Texture));
	}
	if(handle.total>=handle.cap) {
		handle.cap+=MAX_TEX;
		handle.textures = (Texture *)realloc(handle.textures, 
				sizeof(Texture)*handle.cap);
	}
	/*the reason we go over all instead of skipping to 
	 * handle.total is to check for dups*/
	int i;
	for(i=0;i<handle.cap;i++) {
		if(handle.textures[i].ID==NULL) {
			handle.textures[i].ID = calloc(strlen(ID)+1, sizeof(char));
			strcpy(handle.textures[i].ID, ID);
			/*Load stb_image to sg_image*/
			int *w,*h,c=0;
			w = &handle.textures[i].width;
			h = &handle.textures[i].height;
			if(fv) stbi_set_flip_vertically_on_load(1);
			unsigned char *data = stbi_load(path, w,h,&c,4);
			WASSERT(data, "Failed to load image: %s", path);
			handle.textures[i].tex = sg_make_view(&(sg_view_desc){
					.texture.image = sg_make_image(&(sg_image_desc){
						.width = *w,
						.height = *h,
						.pixel_format = SG_PIXELFORMAT_RGBA8,
						.data.mip_levels[0] = PTRRANGE(data, ((*w)*(*h)*4)),
						.label = ID
    		})});
			stbi_image_free(data);
			if(fv) stbi_set_flip_vertically_on_load(0);
			handle.total++;
			char buf[1024];
			sprintf(buf, "Successfully loaded texture[%s]: %s", ID, path);
			WLOG(INFO, buf);
			break;
		} else if(!strcmp(handle.textures[i].ID, ID)) {
			WLOG(WARN, "Duplicate texture ID, skipping...");
		}
	}
}

sg_view getTexture(char *ID) {
	int i;
	for(i=0;i<handle.total;i++) {
		if(handle.textures[i].ID!=NULL&&!strcmp(handle.textures[i].ID,ID)) 
			return handle.textures[i].tex;
	}
	char buf[strlen(ID)+128];
	sprintf(buf, "Failed to find image %s", ID);
	WLOG(WARN, buf);
	return (sg_view){0};
}

void OEGetTextureSize(char *ID, int *x, int *y) {
	int i;
	for(i=0;i<handle.total;i++) {
		if(handle.textures[i].ID!=NULL&&!strcmp(handle.textures[i].ID,ID)) { 
			*x = handle.textures[i].width;
			*y = handle.textures[i].height;
		}
	}
	char buf[strlen(ID)+128];
	sprintf(buf, "Failed to find image %s", ID);
	WLOG(WARN, buf);
}

