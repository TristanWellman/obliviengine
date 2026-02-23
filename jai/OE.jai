/*Copyright (c) 2026 Tristan Wellman
 *
 *  Jai bindings for the OE library.
 *  This includes the bindings for OELights, textures, OEUI, and the Macky config library.
 *  TODO: Just make a generator for this, my ass does not want to manually write it again.
 */

#import "Basic";
#import "Compiler";
#import "SDL";

/* Sokol defs
 * You can also just load https://github.com/colinbellino/sokol-jai/blob/main/sokol/gfx/ */
sg_buffer :: struct {
    id : u32;
}

sg_image :: struct {
    id : u32;
}

sg_sampler :: struct {
    id : u32;
}

sg_shader :: struct {
    id : u32;
}

sg_pipeline :: struct {
    id : u32;
}

sg_view :: struct {
    id : u32;
}

sg_range :: struct {
    ptr : *void;
    size : u64;
}

/*
 * OE defs
 */

vec2 :: [2] float;
vec3 :: [3] float;
vec4 :: [4] float;
mat4x4 :: [4] vec4;

VIEW__texture :: 3;
OE_TEXPOS :: VIEW__texture;
OE_WHITEP :: 0xFFFFFFFF;

OE_USABLE_SCREEN_SPACE :: (1<<1|1);
OE_FULLSCREEN :: (1<<2|1);
OE_FULLSCREEN_DESKTOP :: (1<<3|1);
OE_BORDERLESS :: (1<<4|1);
OE_BORDERED :: (1<<5|1);
OE_LOW_GRAPHICS :: (1<<6|1);
OE_MED_GRAPHICS :: (1<<7|1);
OE_HIGH_GRAPHICS :: (1<<8|1);
OE_KEEPVERTS :: (1<<9|1);

MAXPOSTPASS :: 16;
OEFXAA :: "OEFXAA";
OESSAO :: "OESSAO";
OEBLOOM :: "OEB";
OESSGI :: "OESSGI";
OEDNOISE :: "OEDNOISE";

RENDFUNC :: #type () -> void #foreign OE;
EVENTFUNC :: #type () -> void #foreign OE;
UNILOADER :: #type () -> void #foreign OE;

/*These are contained in OE's util.h so you don't have to pass float arrays around functions.*/
V3SIZE : s32 : 3;
Vec3 :: struct {
	x, y, z: float;
}

Vec2 :: struct {
	x, y: float;
}

Color :: struct {
	r, g, b, a: float;
}

operator *= :: (color: *Color, scalar: float) {
	color.r *= scalar;
	color.g *= scalar;
	color.b *= scalar;
	color.a *= scalar;
}

operator /= :: (color: *Color, scalar: float) {
	color.r /= scalar;
	color.g /= scalar;
	color.b /= scalar;
	color.a /= scalar;
}

/*
 * #define RGBA255TORGBA1(_src) \
 *	 ((__typeof(_src)){(_src.r)/255.0f, (_src.g)/255.0f, \
 *	  (_src.b)/255.0f, (_src.a)/255.0f})
*/
RGBA255TORGBA1 :: (_src: *Color) #expand {
	_src /= 255.0;
}

RGBA1TORGBA255 :: (_src: *Color) #expand {
	_src *= 255.0;
}

face :: enum u32 {
	FRONT :: 0;
	BACKWARD :: 1;
	LEFT :: 2;
	RIGHT :: 3;
	UP :: 4;
	DOWN :: 5;
}

CamType :: enum u32 {
	PERSPECTIVE :: 0;
	ISOMETRIC :: 1;
}

Camera :: struct {
	model: mat4x4;
	view, proj, rotation: mat4x4;
	mvp: mat4x4;
	position, target, up: vec3;
	front, right: vec3;
	ray_hit: vec3;
	aspect: float;
	fov: float;
	oScale: float;
}

Object :: struct {
	vrtPtr: *float;
	vrtSize: *u32;
	// OEScript script
	vbuf, ibuf: sg_buffer;
	defShader: sg_shader;
	pipe: sg_pipeline;
	name: *u8;
	ID: *u8;
	numID: s32;
	numIndices: s32;
	originalModel: mat4x4;
	model: mat4x4;
	pos: vec3;
}

OEInstanceBatch :: struct {
	obj: *Object;
	size: u32;
	positions: *vec3;
	instModelsr0: *vec4;
	instModelsr1: *vec4;
	instModelsr2: *vec4;
	instModelsr3: *vec4;
	vbuf: sg_buffer;
	mbuf0, mbuf1, mbuf2, mbuf3: sg_buffer;
}

Mouse :: struct {
	posx, posy: s32;
	ray_hit: vec3;
}

PostPass :: struct {
	passTime: float;
	ID: *u8;
	pipe: sg_pipeline;
	uniformBind: UNILOADER;
}

OEGetObjectFromName :: (name: *u8) -> *Object #foreign OE;
OEGetInstanceBatchFromName :: (name: *u8) -> *OEInstanceBatch #foreign OE;
OEAttachScript :: (ID: *u8, scriptPath: *u8) #foreign OE;
OECreateInstanceBatch :: (obj: *Object) #foreign OE;
OEPushInstanceBatchData :: (batch: *OEInstanceBatch, pos: *vec3) #foreign OE;
OECreateObject :: (obj: Object) #foreign OE;
OECreateMeshFromAssimp :: (name: *u8, path: *u8, pos: *vec3, flag: s32) #foreign OE;
OESetObjectPosition :: (ID: *u8, position: *vec3) #foreign OE;
OEGetObjectPosition :: (ID: *u8) -> Vec3 #foreign OE;
OERotateObject :: (ID: *u8, deg: float) #foreign OE;
OEResetRotation :: (ID: *u8) #foreign OE;
OEDrawInstanceBatch :: (batch: *OEInstanceBatch) #foreign OE;
OEDrawInstanceBatchTex :: (batch: *OEInstanceBatch, assign: s32, texture: sg_view) #foreign OE;
OEDrawObject :: (obj: *Object) #foreign OE;
OEDrawObjectTex :: (obj: *Object, assign: s32, texture: sg_view) #foreign OE;
OEDrawObjectEx :: (obj: *Object, apply_uniforms: UNILOADER) #foreign OE;
OEGetDefCubeShader :: () -> sg_shader #foreign OE;
OEGetDefInstShader :: () -> sg_shader #foreign OE;
OESetDefaultShader :: (shader: sg_shader) #foreign OE;
OESetDefaultInstancingShader :: (shader: sg_shader) #foreign OE;
OEGetDefaultCubeObj :: (name: *u8) -> Object #foreign OE;
OEGetDefaultTexture :: () -> sg_view #foreign OE;
OEgetSampler :: () -> sg_sampler #foreign OE;
OERendererIsRunning :: () -> s32 #foreign OE;
OEUpdateViewMat :: () #foreign OE;
OEEnableDebugInfo :: () #foreign OE;
OEDisableDebugInfo :: () #foreign OE;
OEDisableSdtc :: () #foreign OE;
OEComputeRotationMatrix :: (out: *mat4x4, front: *vec3, up: *vec3) #foreign OE;
OEDumpSupportedPixelFormats :: () -> s32 #foreign OE;
OEGetGLVersion :: (maj: *s32, min: *s32) #foreign OE;
OEGLFallbackInit :: () #foreign OE;
OESetRenderResolution :: (w: s32, h: s32) #foreign OE;
OEForceGraphicsSetting :: (flag: s32) #foreign OE;
OEInitRenderer :: (width: s32, height: s32, *u8, camType: CamType) #foreign OE;
OEMoveCam :: (direction: face, len: float) #foreign OE;
OECamSet :: (pos: *vec3) #foreign OE;
OEGetCamera :: () -> *Camera #foreign OE;
OEGetCamPos :: () -> Vec3 #foreign OE;
OEGetEvent :: () -> SDL_Event #foreign OE;
OEIsKeyPressed :: () -> s32 #foreign OE;
OEIsKeyRepeating :: () -> s32 #foreign OE;
OEGetKeySym :: () -> s32 #foreign OE;
OEGetMousePos :: (x: *s32, y: *s32) #foreign OE;
OEGetMouse :: () -> Mouse #foreign OE;
OEIsMousePressed :: () -> s32 #foreign OE;
OEIsMouseRepeating :: () -> s32 #foreign OE;
OEGetMouseEvent :: () -> *SDL_MouseButtonEvent #foreign OE;
OESetWindowFullscreen :: () #foreign OE;
OESetWindowFullscreenDesktop :: () #foreign OE;
OESetWindowBorderless :: () #foreign OE;
OESetWindowUsableScreen :: () #foreign OE;
OESetWindowDisplayMode :: (flag: s32) #foreign OE;
OEGetMouseScrollDown :: () -> u32 #foreign OE;
OEGetMouseScrollUp :: () -> u32 #foreign OE;
OEGetGameController :: () -> *SDL_GameController #foreign OE;
OEPollEvents :: (event: EVENTFUNC) #foreign OE;
OEGetFPS :: () -> s32 #foreign OE;
OEGetFrameTime :: () -> float #foreign OE;
OEGetFrameSwap :: () -> u32 #foreign OE;
OEGetTick :: () -> float #foreign OE;
OEGetWindow :: () -> *SDL_Window #foreign OE;
OEGetWindowResolution :: (x: *s32, y: *s32) #foreign OE;
OEGetOEUIData :: () -> *OEUIData #foreign OE;
OEQueryOSInfo :: () -> *u8 #foreign OE;
OEEnableSSAO :: () #foreign OE;
OEDisableSSAO :: () #foreign OE;
OEUpdateBloomParams :: (threshold: float, strength: float) #foreign OE;
OEEnableBloom :: (threshold: float, strength: float) #foreign OE;
OEDisableBloom :: () #foreign OE;
OEEnableFXAA :: () #foreign OE;
OEDisableFXAA :: () #foreign OE;
OEEnableSSGI :: (rays: s32, steps: s32) #foreign OE;
OEUpdateSSGIParams :: (rays: s32, steps: s32) #foreign OE;
OEDisableSSGI :: () #foreign OE;
OEEnableDNOISE :: () #foreign OE;
OEDisableDNOISE :: () #foreign OE;
OESetCursor :: (filePath: *u8) #foreign OE;
OEAddPostPass :: (is: *u8, pipe: sg_pipeline, loader: UNILOADER) -> *PostPass #foreign OE;
OERemovePostPass :: (id: *u8) #foreign OE;
OEGetPostPassTime :: (id: *u8) -> float #foreign OE;
OERenderFrame :: (drawCall: RENDFUNC, cimgui: RENDFUNC, OEUI: RENDFUNC) #foreign OE;
OERendererTimerStart :: () #foreign OE;
OERendererTimerEnd :: () #foreign OE;
OECleanup :: () #foreign OE;
/*I am not going to worry about these for now since Jai has good thread pool handling*/
// OEInitThreadPool
// OEDispatchThread
// OEGetThreadState
// OESetThreadState
// OEWaitThread
// OEDestroyThreadPool
OEClearInstanceBatchData :: () #foreign OE;

/*
 * OE texture defs
 */

Texture :: struct {
	tex: sg_view;
	ID: *u8;
	width, height: s32;
}

textureHandle :: struct {
	textures: *Texture;
	cap: s32;
	total: s32;
}

addTexture :: (ID: *u8, path: *u8, fv: s32) #foreign OE;
getTexture :: (ID: *u8) -> sg_view #foreign OE;
OEGetTextureSize :: (ID: *u8, x: *s32, y: *s32) #foreign OE;

/*
 * OELights defs
 */

MAXLIGHTS :: 64;

OELight :: struct {
	pos: vec3;
	color: Color;
	ID: *u8;
}

OEAddLight :: (ID: *u8, pos: *vec3, color: Color) #foreign OE;
OEDoesLightExist :: (ID: *u8) #foreign OE;
OERemoveLight :: (ID: *u8) #foreign OE;
OESetLightPosition :: (ID: *u8, pos: *vec3) #foreign OE;
OESetLightColor :: (ID: *u8, color: Color) #foreign OE;
getNumLights :: () -> s32 #foreign OE;

/*
 * OEUI defs
 */

OEUI_DEFFONTSIZE :: 32;
OEUI_DEFFONTRESSCALE :: 40;
OEUI_ATLASWID :: 1024;
OEUI_ATLASHEI :: 1024;
OEUI_ATLASSIZE :: (OEUI_ATLASWID*OEUI_ATLASHEI);
OEUI_FSTCHR :: 32;
OEUI_FENDCHR :: 127;
OEUI_STBGLYPHSIZE :: (OEUI_FENDCHR-OEUI_FSTCHR);
OEUI_FATTRPOS :: 0;
OEUI_FATTRTCOORD :: 1;
OEUI_STBGL :: 1;
OEUI_KEEPFBMEM :: (1<<0|1);

OEUI_bakedchar :: struct {
	x0, y0, x1, y1: u16;
	xoff, yoff, xadvance: float;
	xoff2, yoff2: float;
}

OEUIFont :: struct {
	ID, path: *u8;
	fb: *u8;
	fbSize: s64;
	glyph: *OEUI_bakedchar;
	atlasTex: sg_view;
	fontSize: u8;
}

OEUIRect :: struct {
	w, h: s32;
	x0, x1, y0, y1: s32;
}

OEUIData :: struct {
	defaultFont: *OEUIFont;
	fontShader: sg_shader;
	fontPipeline: sg_pipeline;
	fontVbuf, fontIbuf: sg_buffer;
	imageBuf: sg_buffer;
}

OEUIInit :: (data: *OEUIData, file: *u8) #foreign OE;
OEUIDestroyTTFBuffer :: (font: *OEUIFont) #foreign OE;
OEUISetFontSize :: (font: *OEUIFont, size: s32) #foreign OE;
OEUILoadFont :: (filePath: *u8, ID: *u8, flag: s32) -> *OEUIFont #foreign OE;
OEUIRenderText :: (font: *OEUIFont, input: *u8, x: s32, y: s32) #foreign OE;
OEUIDrawImage :: (image: sg_view) #foreign OE;
OEUIDrawImageEx :: (image: sg_view, x: s32, y: s32, w: s32, h: s32) #foreign OE;
