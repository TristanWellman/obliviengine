/*Copyright (c) 2026, Tristan Wellman*/

#include <OE/OE.h>

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

typedef struct {
	SDL_MetalView view;
	CAMetalLayer* layer;
	id<MTLDevice> device;
	id<CAMetalDrawable> drawable;
	id<MTLTexture> depthTexture;
} OEMetalRenderer;

OEMetalRenderer *metalRenderer = NULL;

_OE_COLD sg_swapchain OEGetSwapChainMetal() {
	int w, h;
	OEGetWindowResolution(&w, &h);
    metalRenderer->drawable = [metalRenderer->layer nextDrawable];
	return (sg_swapchain) {
		.sample_count = 1,
		.color_format = SG_PIXELFORMAT_RGBA16F,
		.depth_format = SG_PIXELFORMAT_DEPTH,
		.width = w,
		.height = h,
		.metal = {
			.current_drawable = (__bridge const void*)metalRenderer->drawable,
        	.depth_stencil_texture = (__bridge const void*)metalRenderer->depthTexture,
        	.msaa_color_texture = NULL
		}
	};
}

_OE_COLD sg_environment OEGetEnvMetal() {
	return (sg_environment) {
		.defaults = {
			.color_format = SG_PIXELFORMAT_RGBA16F,
			.depth_format = SG_PIXELFORMAT_DEPTH,
			.sample_count = 1
		},
		.metal.device = (__bridge const void*)metalRenderer->device
	};
} 

void OEInitMetalRenderer(int w, int h, char *title) {
	metalRenderer = calloc(1, sizeof(OEMetalRenderer));
	 _OESetWindow(SDL_CreateWindow(
			title,
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			w, h,
			SDL_WINDOW_METAL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE));
	SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_STEAMDECK, "1");

	metalRenderer->device = MTLCreateSystemDefaultDevice();
    metalRenderer->view = SDL_Metal_CreateView(OEGetWindow());
    metalRenderer->layer = SDL_Metal_GetLayer(metalRenderer->view);
    metalRenderer->layer.device = metalRenderer->device;
    metalRenderer->layer.magnificationFilter = kCAFilterNearest;
    metalRenderer->layer.framebufferOnly = true;	
	metalRenderer->layer.pixelFormat = MTLPixelFormatRGBA16Float;
	CGColorSpaceRef srgbSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
	metalRenderer->layer.colorspace = srgbSpace;
	CGColorSpaceRelease(srgbSpace);

	MTLTextureDescriptor *depthDescriptor = [MTLTextureDescriptor 
		texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
		width:w
    	height:h
        mipmapped:NO];
    depthDescriptor.storageMode = MTLStorageModePrivate;
    depthDescriptor.usage = MTLTextureUsageRenderTarget;
    metalRenderer->depthTexture = [metalRenderer->device newTextureWithDescriptor:depthDescriptor];
}

void OEAdjustResolutionMetal(int w, int h) {
	MTLTextureDescriptor *depthDescriptor = [MTLTextureDescriptor 
		texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
		width:w
    	height:h
        mipmapped:NO];
    depthDescriptor.storageMode = MTLStorageModePrivate;
    depthDescriptor.usage = MTLTextureUsageRenderTarget;
    metalRenderer->depthTexture = [metalRenderer->device newTextureWithDescriptor:depthDescriptor];
}
