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

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#if !defined CIMGUI_USE_SDL2 && !defined CIMGUI_USE_OPENGL3
#define CIMGUI_USE_OPENGL3
#define CIMGUI_USE_SDL2
#endif
#include <cimgui/cimgui.h>
#include <cimgui/cimgui_impl.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>

#include "linmath.h"
#include "util.h"
#include "texture.h"
#include "meshParse.h"
#include "macky.h"
#include "OELights.h"
#include "log.h"
#include "OEScript.h"

#include "simple.glsl.h"
#include "quad.glsl.h"
#include "fxaa.glsl.h"
#include "bloom.glsl.h"
#include "ssao.glsl.h"
#include "rayTracer.glsl.h"
#include "ssgi.glsl.h"
#include "denoise.glsl.h"

#define MAXOBJS 1000000

#define OE_TEXPOS (IMG__texture)

#define OE_WHITEP (0xFFFFFFFF)

#define MAXPOSTPASS 16
#define OEFXAA "OEFXAA"
#define OESSAO "OESSAO"
#define OEBLOOM "OEB"
#define OESSGI "OESSGI"
#define OEDNOISE "OEDNOISE"

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
	/*These need to stay up top for alignment*/
	mat4x4 model;
	mat4x4 view, proj, rotation;
	mat4x4 mvp;

	vec3 position, target, up;
	vec3 front, right; /* front could also be called direction! */
	vec3 ray_hit; /*This is what the camera is looking at (good for player position)*/


	/*These must stay last for the 4x4 alignment (sort of a padding)*/
	float aspect;
	float fov;
	float oScale;
} Camera;

typedef struct {
	OEScript script;
	sg_buffer vbuf, ibuf;
	sg_shader defShader;
	/*Each obj get's it's own pipe so we can draw one defined obj multiple times*/	
	sg_pipeline pipe;
	char *name;
	char *ID;
	int numID;

	int numIndices;

	mat4x4 originalModel; /*For user to reset the model matrix*/
	mat4x4 model;

	vec3 pos;
} Object;

/*Depth buffer and related things should not be in an SSAO struct, 
 * but I am currently too lazy to go through and fix every instance of it*/
typedef struct {
	int w,h;
	sg_shader depthShader;	

	sg_shader ssaoShader;
	sg_image ssaoBuffer;
	sg_image finalImage; /*resolv*/
	sg_pipeline pipe;
	sg_attachments atts;
	sg_sampler sampler;
	OESSAO_params_t params;
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

typedef struct {
	char *ID;
	sg_pipeline pipe;
	UNILOADER uniformBind;
} PostPass;

struct OEImgui {
	ImGuiIO *ioptr;	
};

typedef struct {
	sg_shader fxaa, ssao, bloom, 
			  ssgi, dnoise;
	sg_pipeline fxaap, ssaop, bloomp, 
				ssgip, dnoisep;
} OEPPShaders;

/*TODO: Seperate a lot of this into different structs so it's not so fat.*/
struct renderer {

	Window *window;
	struct OEImgui imgui;
	SDL_Event event;
	int keyPressed;
	int lastKey;

	Camera cam;
	enum CamType camType;
	Mouse mouse;

	sg_bindings bindings;
	Object *objects;
	int objCap;
	int objSize;
	OEPPShaders ppshaders; /*These are the enable/disable-able post-pass shaders*/
	sg_shader defCubeShader;
	sg_shader rayTracedShader;

	/*This is for if you call a draw function and do not pass a texture*/
	sg_image defTexture;

	sg_image depthDummy; /*trick sokol for depth testing*/
	sg_image depthBuffer;
	sg_image normalBuffer;
	sg_image positionBuffer;
	sg_image noiseBuffer;
	sg_image prevFrameBuffer;

	/*Render texture, and ssao buffer stuff*/
	sg_image renderTarget;
	sg_attachments renderTargetAtt;
	sg_image postTarget;
	sg_image postTargetPong; /*We must alternate images/attachments for user passes*/
	sg_attachments postTargetAtt;
	sg_attachments postTargetAttPong;
	sg_attachments prevFrameTarg;
	/*This is just a shader for the screen quad*/
	sg_shader renderTargetShade;
	sg_pipeline renderTargetPipe;
	sg_buffer renderTargetBuff;
	
	SSAO ssao;

	/*The user's post passes (bloom, fxaa, ssao, etc.)*/
	PostPass postPasses[MAXPOSTPASS];
	int postPassSize;

	OEBloom_params_t bloomParams;
	OESSGI_params_t ssgiParams;
	OEDNOISE_params_t deNoiseParams;

	OELuaData luaData; 

	int debug;
	float tick;
	float frameTime;
	float fps;
	int frame_start, frame_end;
};

static struct renderer *globalRenderer;

/*objs*/
Object *OEGetObjectFromName(char *name);
void OEAttachScript(char *ID, char *scriptPath);
void OECreateObject(Object obj);
void OECreateObjectEx(char *name, vec3 pos,
		sg_buffer_desc vbuf, sg_buffer_desc ibuf, sg_shader defShader,
		sg_pipeline_desc pipe);
/*For now mesh objects require the default shader.
 * Hopefully I change this in the future for customization but idk :| */
void OECreateObjectFromMesh(OEMesh *mesh, vec3 pos);
/*For loading things other than .obj files*/
void OECreateMeshFromAssimp(char *name, char *path, vec3 pos);
void OESetObjectPosition(char *ID, vec3 position);
Vec3 OEGetObjectPosition(char *ID);
/*Rotates obj around the y-axis TODO: specify axis of rotation*/
void OERotateObject(char *ID, float deg);
void OEResetRotation(char *ID);

void OEDrawObject(Object *obj);
void OEDrawObjectTex(Object *obj, int assign, sg_image texture);
void OEDrawObjectEx(Object *obj, UNILOADER apply_uniforms);
void OEDrawObjectTexEx(Object *obj, int assign,
		sg_image texture, UNILOADER apply_uniforms); 

sg_buffer_desc OEGetCubeVertDesc();
sg_buffer_desc OEGetCubeIndDesc();
sg_shader OEGetDefCubeShader();
sg_shader OEGetRayTracedShader();
sg_pipeline_desc OEGetRayTracedPipe();
void OESetDefaultShader(sg_shader shader);
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
void OECamSet(vec3 pos);
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

void OEEnableSSAO();
void OEDisableSSAO();
void OEUpdateBloomParams(float threshold, float strength);
void OEEnableBloom(float threshold, float strength);
void OEDisableBloom();
void OEEnableFXAA();
void OEDisableFXAA();
void OEEnableSSGI();
void OEDisableSSGI();
void OEEnableDNOISE();
void OEDisableDNOISE();
PostPass *OEAddPostPass(char *id, sg_pipeline pipe, UNILOADER loader);
void OERemovePostPass(char *id);
void OERenderFrame(RENDFUNC drawCall, RENDFUNC cimgui);
void OERendererTimerStart();
void OERendererTimerEnd();
void OECleanup(void);

#endif
