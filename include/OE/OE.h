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
#define OBJSTEP 100

#define MAXDRAWCALLS 500000
#define DRAWCALLSTEP 10 /*This is how much we realloc to the queue*/

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

struct DrawCall {
	Object *obj;
};

typedef struct {
	int cap, size;
	struct DrawCall *drawCalls;
} DrawCallQueue;

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
	int keyPressed :1;
	int lastKey :8;

	Camera cam;
	enum CamType camType;
	Mouse mouse;

	sg_bindings bindings;
	Object *objects;
	int objCap :20;
	int objSize :20;
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
	int postPassSize :4;

	OEBloom_params_t bloomParams;
	OESSGI_params_t ssgiParams;
	OEDNOISE_params_t deNoiseParams;

	OELuaData luaData; 

	DrawCallQueue drawQueue;

	int debug :1;
	float tick;
	float frameTime;
	float fps;
	int frame_start, frame_end;
};

static struct renderer *globalRenderer;

/*objs*/
/**
 * @brief Get an Object's pointer by providing the ID.
 *
 * @param name The name/ID for the Object you are trying to retrieve.
 * */
Object *OEGetObjectFromName(char *name);
/**
 * @brief Adds a Lua script to an Object, the script will run before every frame.
 *
 * @param ID The name/ID for the Object you are attaching the script to.
 * @param scriptPath The file path to the designated Lua script.
 * */
void OEAttachScript(char *ID, char *scriptPath);
/**
 * @brief Adds a new Object to Obliviengine, this Object is accessible via OEGetObjectFromName
 *
 * @param obj The Object at which you are adding.
 * */
void OECreateObject(Object obj);
/**
 * @brief Adds new Object to Obliviengine, this contains more customizable parameters. 
 * This object is accessible via OEGetObjectFromName.
 *
 * @param name The name of the Object.
 * @param pos The world space position of the Object.
 * @param vbuf The vertex buffer for the Object.
 * @param ibuf The index buffer for the Object.
 * @param defShader The default shader to use on this Object.
 * @param pipe The default pipeline to use on this Object.
 * */
void OECreateObjectEx(char *name, vec3 pos,
		sg_buffer_desc vbuf, sg_buffer_desc ibuf, sg_shader defShader,
		sg_pipeline_desc pipe);
/*For now mesh objects require the default shader.
 * Hopefully I change this in the future for customization but idk :| */
/**
 * @brief Adds a new Object to Obliviengine by parsing a mesh (usually from an object file).
 *
 * @param mesh The mesh struct holding all the model data.
 * @param pos The world space position for the Object.*/
void OECreateObjectFromMesh(OEMesh *mesh, vec3 pos);
/*For loading things other than .obj files*/
/**
 * @brief Adds a new Object to Obliviengine via Assimp for more complex model file formats.
 *
 * @param name The name/ID for the Object.
 * @param path The file path for the model file.
 * @param pos The world space position of the Object.
 * */
void OECreateMeshFromAssimp(char *name, char *path, vec3 pos);
/**
 * @brief Sets the world space position of an existing OE Object.
 *
 * @param ID The name/ID of the Object you are setting the position of.
 * @param position The world space position you'd like to move the Object to.
 * */
void OESetObjectPosition(char *ID, vec3 position);
/**
 * @brief Retrieve the current set world space position of an Object.
 *
 * @param ID The name/ID of the Object at which you are obtaining the position.
 * */
Vec3 OEGetObjectPosition(char *ID);
/*Rotates obj around the y-axis TODO: specify axis of rotation*/
/**
 * @brief Set the Y-axis rotation angle of an Object.
 *
 * @param ID The name/ID of the Object at which you are setting the rotation.
 * @param deg The angle in degrees that is used to set the Object's rotation.
 * */
void OERotateObject(char *ID, float deg);
/**
 * @brief Resets an Object's rotation angle to 0.
 *
 * @param ID The name/ID of the Object you are resetting.
 * */
void OEResetRotation(char *ID);

/**
 * @brief Sends a draw call to the GPU for a specified object.
 *
 * @param obj The Object you are drawing.
 * */
void OEDrawObject(Object *obj);
/**
 * @brief Sends a draw call to the GPU with a specified Object and texture.
 *
 * @param obj The Object you are drawing.
 * @param assign The texture index for the Object.
 * @param texture The texture image.
 * */
void OEDrawObjectTex(Object *obj, int assign, sg_image texture);
/**
 * @brief Sends a draw call to the GPU with a specified Object and custom uniform application.
 *
 * @param obj The Object you are drawing.
 * @param apply_uniforms the function for applying your custom uniforms.
 * */
void OEDrawObjectEx(Object *obj, UNILOADER apply_uniforms);
/**
 * @brief Sends a draw call to the GPU with a specified Object, Texture, and uniform application.
 *
 * @param obj The Object you are drawing.
 * @param assign The texture index for the Object.
 * @param texture The texture image.
 * @param apply_uniforms The function for applying your custom uniforms.
 * */
void OEDrawObjectTexEx(Object *obj, int assign,
		sg_image texture, UNILOADER apply_uniforms); 
/**
 * @brief Gets the Sokol buffer for the OE Cube vertices.
 * */
sg_buffer_desc OEGetCubeVertDesc();
/**
 * @brief Gets the Sokol buffer for the OE Cube indices.
 * */
sg_buffer_desc OEGetCubeIndDesc();
/**
 * @brief Gets the default OE cube shader (simple.glsl)
 * */
sg_shader OEGetDefCubeShader();
/**
 * @brief Gets the OE ray tracing shader (not implemented yet).
 * */
sg_shader OEGetRayTracedShader();
/**
 * @brief Gets the OE ray tracing pipeline (not implemented yet).
 * */
sg_pipeline_desc OEGetRayTracedPipe();
/**
 * @brief Set the default OE shader, this will overrite from simple.glsl.
 *
 * @param shader The shader you are setting as default.
 * */
void OESetDefaultShader(sg_shader shader);
/**
 * @brief Creates and returns a new OE cube object.
 * */
Object OEGetDefaultCubeObj(char *name);
/**
 * @brief Gets the default OE texture, this is a full white image.
 * */
sg_image OEGetDefaultTexture(); 

/*renderer*/
/**
 * @brief Tells if the renderer is running or not.
 * */
int OERendererIsRunning();
/**
 * @brief Get the default pipeline for OE, this is for the simple.glsl format
 * (position, color, normal, uv).
 *
 * @param shader The shader you want to use in the pipeline.
 * @param label The name/ID for the pipeline.
 * */
sg_pipeline_desc OEGetDefaultPipe(sg_shader shader, char *label);
/**
 * @brief The default screen pipeline (this is for things like post processing shaders).
 *
 * @param shader The shader you want to use in the pipeline.
 * @param label The name/ID for the pipeline.
 * */
sg_pipeline_desc OEGetQuadPipeline(sg_shader shader, char *label);
/**
 * @brief Get the Sokol/OpenGL environment.
 * */
sg_environment OEGetEnv();
/**
 * @brief Get the Sokol/OpenGL swapchain.
 * */
sg_swapchain OEGetSwapChain(); 
/**
 * @brief Internally updates the Camera's view, projection, target, etc.
 * */
void OEUpdateViewMat();
/**
 * @brief This enables debugging information onto the screen/terminal. 
 * Shows things like FPS, FrameTime, Camera Position, etc.
 * */
void OEEnableDebugInfo();
/**
 * @brief Gets a rotation matrix from a front and up vector.
 *
 * @param out The 4x4 matrix to hold the output.
 * @param front The front vector.
 * @param up The up vector.
 * */
void OEComputeRotationMatrix(mat4x4 out, vec3 front, vec3 up);
/**
 * @brief This initializes the OE renderer (Sokol, SDL2, OpenGL), camera, and other renderer Objects.
 *
 * @param width The width in pixels for the window.
 * @param height The height in pixels for the window.
 * @param title The title of the window.
 * @param camType The type of camera projection you want (PERSPECTIVE, ISOMETRIC).
 * */
void OEInitRenderer(int width, int height, char *title, enum CamType camType);
/**
 * @brief Move the camera Forward, Backward, Up, Down, Left, and Right.
 *
 * @param direction The direction of movement.
 * @param len The distance you are moving the camera.
 * */
void OEMoveCam(enum face direction, float len);
/**
 * @brief Set the world space position of the camera.
 *
 * @param pos The world space position vector.
 * */
void OECamSet(vec3 pos);

/**
 * @brief Get the pointer to the OE camera structure.
 * */
Camera *OEGetCamera();
/**
 * @brief Get the world space position of the OE camera in a Vec3.
 * */
Vec3 OEGetCamPos(); 
/**
 * @brief Get the SDL event handler.
 * */
SDL_Event OEGetEvent();
/**
 * @brief Check if a key is being pressed.
 * */
int OEIsKeyPressed();
/**
 * @brief Get the SDL key handle.*/
int OEGetKeySym();
/**
 * @brief Get the current mouse position in pixel space relative to the window.
 *
 * @param x A pointer to the x-axis mouse location.
 * @param y A pointer to the y-axis mouse location.
 * */
void OEGetMousePos(int *x, int *y);
/**
 * @brief Get the structure for mouse data, the position, and the world space ray hit.
 * */
Mouse OEGetMouse();
/**
 * @brief Runs the SDL event polling loop along with a user defined key handler.
 *
 * @param event The user's event handling function.
 * */
void OEPollEvents(EVENTFUNC event);
/**
 * @brief Get the FPS
 * */
int OEGetFPS();
/**
 * @brief Get the frame time.
 * */
float OEGetFrameTime();
/**
 * @brief This gets the current tick, not tick speed.
 * */
float OEGetTick();
/**
 * @brief Get the pointer to the OE SDL window.
 * */
SDL_Window *OEGetWindow(); 

/**
 * @brief Enables the Screen Space Ambient Occlusion shader.
 * */
void OEEnableSSAO();
/**
 * @brief Disables SSAO.
 * */
void OEDisableSSAO();
/**
 * @brief Change the settings for the bloom shader (strength and threshold).
 *
 * @param threshold How bright something needs to be before bloom activates.
 * @param strength How bright the bloom is.
 * */
void OEUpdateBloomParams(float threshold, float strength);
/**
 * @brief Enables the bloom shader.
 *
 * @param threshold How bright something needs to be before bloom activates.
 * @param strength How bright the bloom is.
 * */
void OEEnableBloom(float threshold, float strength);
/**
 * @brief Disable the bloom shader.
 * */
void OEDisableBloom();
/**
 * @brief Enable the OE anti-aliasing shader.
 * */
void OEEnableFXAA();
/**
 * @brief Disable the FXAA shader.
 * */
void OEDisableFXAA();
/**
 * @brief Enable the OE Screen Space Global Illumination shader.
 * low: rays = 16 steps = 8
 * med: rays = 32 steps = 16
 * high: rays = 64 steps = 32
 *
 * @param rays The amount of rays to cast per pixel.
 * @param steps The amount of steps (distance) for a ray to compute.
 * */
void OEEnableSSGI(int rays, int steps);
/**
 * @brief Update the rays and steps for SSGI without having to re-init the shader.
 *
 * @param rays The amount of rays to cast per pixel.
 * @param steps The amount of steps (distance) for a ray to compute.
 * */
void OEUpdateSSGIParams(int rays, int steps);
/**
 * @brief Disable the SSGI shader.
 * */
void OEDisableSSGI();
/**
 * @brief Enable the de-noising shader, this is automatically applied in the SSGI shader.
 * */
void OEEnableDNOISE();
/**
 * @brief Disable the de-noise shader.
 * */
void OEDisableDNOISE();
/**
 * @brief Add a post-pass shader, this runs a shader on the current frame.
 *
 * @param id The name/ID of your post-pass, I.E. Bloom.
 * @param pipe The pipeline to use for the shader, usually can just be the OEQuad pipeline.
 * @param loader The custom uniform loader function for your shader.
 * */
PostPass *OEAddPostPass(char *id, sg_pipeline pipe, UNILOADER loader);
/**
 * @brief Remove a post-pass shader.
 *
 * @param id The name/ID of the post-pass you are removing.
 * */
void OERemovePostPass(char *id);
/**
 * @brief Runs the actual rendering process, all your draw calls, UI, etc.
 *
 * @param drawCall The user defined function for drawing objects.
 * @param cimgui The user defined function for rendering a cimgui UI.
 * */
void OERenderFrame(RENDFUNC drawCall, RENDFUNC cimgui);
/**
 * @brief This starts the frame timer, this is useful for the OpenXR renderer or a user renderer.
 * */
void OERendererTimerStart();
/**
 * @brief Ends the frame timer.
 * */
void OERendererTimerEnd();
/**
 * @brief Cleans up and Frees OE before exit.
 * */
void OECleanup();

#endif
