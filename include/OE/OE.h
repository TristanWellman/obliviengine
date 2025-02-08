/*Copyright (c) 2025, Tristan Wellman*/
#ifndef RENDERER_H
#define RENDERER_H

#define SOKOL_NO_ENTRY
#define SOKOL_EXTERNAL_GL_LOADER
#include <glad/glad.h>
#include <sokol/sokol_gfx.h>
#include <sokol/sokol_log.h>
#include <sokol/util/sokol_debugtext.h>

#if defined __WIN32__ || __WIN64__
#define SDL_MAIN_HANDLED
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_syswm.h>

#include "linmath.h"
#include "util.h"
#include "texture.h"
#include "meshParse.h"
#include "macky.h"
#include "OELights.h"
#include "log.h"

#include "simple.glsl.h"
#include "quad.glsl.h"

#define MAXOBJS 1000000

#define OE_TEXPOS (IMG__texture)

#define OE_WHITEP (0xFFFFFFFF)

/*This is here because SG_RANGE does not work with ptr sizes.*/
#define PTRRANGE(ptr_, size_) \
	(sg_range){ .ptr = (ptr_), .size = sizeof(*(ptr_)) * (size_) }

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

	/*This is for if you call a draw function and do not pass a texture*/
	sg_image defTexture;

	/*Render texture, and ssao buffer stuff*/
	sg_image renderTarget;
	sg_attachments renderTargetAtt;
	/*This is just a shader for the screen quad*/
	sg_shader renderTargetShade;
	sg_pipeline renderTargetPipe;
	sg_buffer renderTargetBuff;
	/*Haven't setup ssao shader yet but we are using the depth buffer from it*/
	SSAO ssao;

	int debug;
	float tick;
	float frameTime;
	float fps;
	int frame_start, frame_end;
};

static struct renderer *globalRenderer;

/*objs*/
Object *OEGetObjectFromName(char *name);
void OECreateObject(Object obj);
void OECreateObjectEx(char *name, vec3 pos,
		sg_buffer_desc vbuf, sg_buffer_desc ibuf, sg_shader defShader,
		sg_pipeline_desc pipe);
/*For now mesh objects require the default shader.
 * Hopefully I change this in the future for customization but idk :| */
void OECreateObjectFromMesh(OEMesh *mesh, vec3 pos
		/*sg_shader defShader, sg_pipeline_desc pipe*/);
void OESetObjectPosition(char *ID, vec3 position); 

void OEDrawObject(Object *obj);
void OEDrawObjectTex(Object *obj, int assign, sg_image texture);
void OEDrawObjectEx(Object *obj, UNILOADER apply_uniforms);
void OEDrawObjectTexEx(Object *obj, int assign,
		sg_image texture, UNILOADER apply_uniforms); 

sg_buffer_desc OEGetCubeVertDesc();
sg_buffer_desc OEGetCubeIndDesc();
sg_shader OEGetDefCubeShader();
Object OEGetDefaultCubeObj(char *name);
sg_image OEGetDefaultTexture(); 

/*renderer*/
int OERendererIsRunning();
sg_pipeline_desc OEGetDefaultPipe(sg_shader shader, char *label);
sg_pipeline_desc OEGetQuadPipeline(sg_shader shader, char *label);
sg_environment OEGetEnv(void);
sg_swapchain OEGetSwapChain(void); 

void OEUpdateViewMat();
void OEEnableDebugInfo();
void OEComputeRotationMatrix(mat4x4 out, vec3 front, vec3 up);
void OEInitRenderer(int width, int height, char *title, enum CamType camType);
void OEMoveCam(enum face direction, float len);
void OECamSet(vec3 pos);

Camera *OEGetCamera();
Vec3 OEGetCamPos(); 
SDL_Event OEGetEvent();
int OEIsKeyPressed();
int OEGetKeySym();
void OEGetMousePos(int *x, int *y);
Mouse OEGetMouse();
void OEPollEvents(EVENTFUNC event);
int OEGetFPS();
float OEGetFrameTime();
float OEGetTick();
SDL_Window *OEGetWindow(); 

void OERenderFrame(RENDFUNC drawCall);
void OERendererTimerStart();
void OERendererTimerEnd();
void OECleanup(void);

#endif
