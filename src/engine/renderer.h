/*Copyright (c) 2025, Tristan Wellman*/
#ifndef RENDERER_H
#define RENDERER_H

#define SOKOL_NO_ENTRY
#define SOKOL_EXTERNAL_GL_LOADER
#include <glad/glad.h>
#include <sokol/sokol_gfx.h>
//#include <sokol/sokol_app.h>
#include <sokol/sokol_log.h>
#include <sokol/util/sokol_debugtext.h>
//#include <sokol/sokol_glue.h>

#if defined __WIN32__ || __WIN64__
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_syswm.h>

#include "linmath.h"
#include "util.h"
#include "texture.h"

#define MAXOBJS 1000000

enum face {
	FRONT,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

enum CamType {
	PERSPECTIVE,
	ISOMETRIC
};

typedef void (*RENDFUNC)();
typedef void (*EVENTFUNC)();
typedef void (*UNILOADER)();

typedef struct {
	float aspect, fov;
	mat4x4 model;
	mat4x4 view, proj, rotation;
	mat4x4 iso;
	mat4x4 mvp;
	vec3 position, target, up;
	vec3 front, right; /* front could also be called direction! */
	vec3 ray_hit; /*This is what the camera is looking at (good for player position)*/
} Camera;

typedef struct {
	sg_buffer vbuf, ibuf;
	sg_shader defShader;
	/*Each obj get's it's own pipe so we can draw one defined obj multiple times*/	
	sg_pipeline pipe;
	char *name;
	char *ID;
	int numID;

	int numIndices;
	mat4x4 model;

	vec3 pos;
} Object;

/*
 * SSAO is not automatically applied to a process!
 * You must add a sampler to your shader and make a pipe to pass the ssao buffers.
 * */
typedef struct {
	int w,h;
	sg_shader depthShader;
	sg_shader ssaoShader;
	sg_image depthBuffer;
	sg_image ssaoBuffer;
	sg_image finalImage; /*resolv*/
	sg_pipeline pipe;
	sg_attachments atts;
	sg_sampler sampler;
} SSAO;

typedef struct {
	int width, height;
	char *title;
	SDL_Window *window;
	SDL_GLContext gl_context;
	int running;
} Window;

typedef struct {
	int posx, posy;
	vec3 ray_hit;
} Mouse;

struct renderer {

	Window *window;
	SDL_Event event;
	int keyPressed;
	int lastKey;

	Camera cam;
	Mouse mouse;

	sg_bindings bindings;
	Object *objects;
	int objCap;
	int objSize;
	sg_shader defCubeShader;

	SSAO ssao;

	int debug;
	float tick;
	float frameTime;
	float fps;
	int frame_start, frame_end;
};

static struct renderer *globalRenderer;

//objs
Object *getObjectFromName(char *name);
void createObject(Object obj);
void createObjectEx(char *name, vec3 pos,
		sg_buffer_desc vbuf, sg_buffer_desc ibuf, sg_shader defShader,
		sg_pipeline_desc pipe);
void setObjectPosition(char *ID, vec3 position); 

void drawObject(Object *obj);
void drawObjectTex(Object *obj, int assign, sg_image texture);
void drawObjectEx(Object *obj, UNILOADER apply_uniforms);
void drawObjectTexEx(Object *obj, int assign,
		sg_image texture, UNILOADER apply_uniforms); 

sg_buffer_desc getCubeVertDesc();
sg_buffer_desc getCubeIndDesc();
sg_shader getDefCubeShader();
Object getDefaultCubeObj(char *name);

//renderer
int rendererIsRunning();
sg_pipeline_desc getDefaultPipe(sg_shader shader, char *label);
sg_environment getEnv(void);
sg_swapchain getSwapChain(void); 

void updateViewMat();
void enableDebugInfo();
void computeRotationMatrix(mat4x4 out, vec3 front, vec3 up);
void initRenderer(int width, int height, char *title, enum CamType camType);
void moveCam(enum face direction, float len);
void camSet(vec3 pos);

Camera *getCamera();
Vec3 getCamPos(); 
SDL_Event getEvent();
int isKeyPressed();
int getKeySym();
void getMousePos(int *x, int *y);
Mouse getMouse();
void pollEvents(EVENTFUNC event);
int getFPS();
float getFrameTime();
float getTick();
SDL_Window *getWindow(); 

void renderFrame(RENDFUNC drawCall);
void rendererTimerStart();
void rendererTimerEnd();
void cleanup(void);

#endif
