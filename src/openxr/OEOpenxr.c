/*Copyright (c) 2025 Tristan Wellman*/
#define SDL_VIDEO_DRIVER_WINDOWS
#include <OE/openxr/OEOpenxr.h>

/*This is not in the header for possible header recomp time savings.*/
#define XR_USE_GRAPHICS_API_OPENGL
#define XR_USE_PLATFORM_WIN32
#include <windows.h>
#include <unknwn.h>
#include <openxr.h>
#include <openxr_platform.h>


XrInstance xrInstance;
XrSystemId xrSystemId;
XrSession xrSession;
XrSwapchain xrSwapchains[NUM_EYES];
XrSpace xrSpace;
XrSessionState xrSessionState = XR_SESSION_STATE_UNKNOWN;

sg_image depth_images[NUM_EYES][MAX_SWAPCHAIN_IMAGES];
sg_image color_images[NUM_EYES][MAX_SWAPCHAIN_IMAGES];
sg_attachments passes[NUM_EYES][MAX_SWAPCHAIN_IMAGES];
uint32_t swapchain_lengths[NUM_EYES]; 
uint32_t swapchain_width;
uint32_t swapchain_height;

void createSwapChains() {
    XrViewConfigurationView viewConfigs[2] = { {XR_TYPE_VIEW_CONFIGURATION_VIEW}, 
											   {XR_TYPE_VIEW_CONFIGURATION_VIEW} };
    uint32_t viewCount = 0;
    xrEnumerateViewConfigurationViews(xrInstance, xrSystemId, 
			XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 2, &viewCount, viewConfigs);

	int eye;
    for(eye=0;eye<2;eye++) {
        XrSwapchainCreateInfo swapchainCreateInfo = { XR_TYPE_SWAPCHAIN_CREATE_INFO,
        						.arraySize = 1,
        						.format = GL_SRGB8_ALPHA8,
        						.width = viewConfigs[eye].recommendedImageRectWidth,
        						.height = viewConfigs[eye].recommendedImageRectHeight,
        						.mipCount = 1, .faceCount = 1,
        						.sampleCount = viewConfigs[eye].recommendedSwapchainSampleCount,
        						.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT |
											  XR_SWAPCHAIN_USAGE_SAMPLED_BIT };

        XrSwapchain swapchain;
        xrCreateSwapchain(xrSession, &swapchainCreateInfo, &swapchain);
        xrSwapchains[eye] = swapchain;

        swapchain_width = swapchainCreateInfo.width;
        swapchain_height = swapchainCreateInfo.height;

        uint32_t imageCount = 0;
        xrEnumerateSwapchainImages(swapchain, 0, &imageCount, NULL);

        swapchain_lengths[eye] = imageCount; 

        XrSwapchainImageOpenGLKHR swapchainImages[MAX_SWAPCHAIN_IMAGES];
        int i;
		for(i=0;i<imageCount;i++) {
            swapchainImages[i].type = XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR;
            swapchainImages[i].next = NULL;
            swapchainImages[i].image = 0;
        }

        xrEnumerateSwapchainImages(swapchain, imageCount, &imageCount, (XrSwapchainImageBaseHeader*)swapchainImages);

        /*create a depth & color image for sg attachments.*/
        for(i=0;i<imageCount;i++) {
 			sg_image_desc color_desc = {
                .render_target = true,
                .width = swapchainCreateInfo.width,
                .height = swapchainCreateInfo.height,
                .pixel_format = SG_PIXELFORMAT_RGBA32F,
//#if defined(SOKOL_GLCORE33)
                .gl_textures = swapchainImages[i].image,
//#endif
                .label = "color_image",
            };
            color_images[eye][i] = sg_make_image(&color_desc);

            sg_image_desc depth_desc = {
                .render_target = true,
                .width = swapchainCreateInfo.width,
                .height = swapchainCreateInfo.height,
                .pixel_format = SG_PIXELFORMAT_DEPTH,
                .label = "depth_image",
            };
            depth_images[eye][i] = sg_make_image(&depth_desc);

            passes[eye][i] = sg_make_attachments(&(sg_attachments_desc){
            		.colors[0].image = color_images[eye][i],
					.depth_stencil.image = depth_images[eye][i]
        		});
        }
    }
}



void initOpenXR() {
    XrInstanceCreateInfo instanceCreateInfo = { XR_TYPE_INSTANCE_CREATE_INFO };
    strcpy(instanceCreateInfo.applicationInfo.applicationName, "ObliviEngineVR");
    instanceCreateInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;

    const char* extensions[] = { XR_KHR_OPENGL_ENABLE_EXTENSION_NAME };
    instanceCreateInfo.enabledExtensionCount = 1;
    instanceCreateInfo.enabledExtensionNames = extensions;

    WASSERT(!XR_FAILED(xrCreateInstance(&instanceCreateInfo, &xrInstance)),
        "Failed to create OpenXR instance\n");

    XrSystemGetInfo systemInfo = { XR_TYPE_SYSTEM_GET_INFO };
    systemInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

    WASSERT(!XR_FAILED(xrGetSystem(xrInstance, &systemInfo, &xrSystemId)),
		"Failed to get OpenXR system\n");

    PFN_xrGetOpenGLGraphicsRequirementsKHR pfnGetOpenGLGraphicsRequirementsKHR = NULL;
    xrGetInstanceProcAddr(xrInstance, "xrGetOpenGLGraphicsRequirementsKHR",
                          (PFN_xrVoidFunction*)&pfnGetOpenGLGraphicsRequirementsKHR);

    XrGraphicsRequirementsOpenGLKHR graphicsRequirements = { XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR };
    WASSERT(!XR_FAILED(pfnGetOpenGLGraphicsRequirementsKHR(xrInstance, xrSystemId, &graphicsRequirements)),
        "Failed to get OpenGL graphics requirements\n");

	/*This could not have been more of a pain to figure out*/
	WLOG(INFO, SDL_GetCurrentVideoDriver());
	SDL_SysWMinfo info_;
	SDL_VERSION(&info_.version);
	WASSERT(SDL_GetWindowWMInfo(OEGetWindow(), &info_),
    	"SDL_GetWindowWMInfo failed: %s", SDL_GetError());

    XrGraphicsBindingOpenGLWin32KHR graphicsBinding = {
        .type = XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR,
        .next = NULL,
        .hDC = GetDC(info_.info.win.window),
        .hGLRC = wglGetCurrentContext()
    };

    XrSessionCreateInfo sessionCreateInfo = { XR_TYPE_SESSION_CREATE_INFO };
    sessionCreateInfo.next = &graphicsBinding;
    sessionCreateInfo.systemId = xrSystemId;

    WASSERT(!XR_FAILED(xrCreateSession(xrInstance, &sessionCreateInfo, &xrSession)),
        "Failed to create OpenXR session\n");

    XrReferenceSpaceCreateInfo spaceCreateInfo = { XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
    spaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_STAGE;
    spaceCreateInfo.poseInReferenceSpace.orientation.w = 1.0f;

    WASSERT(!XR_FAILED(xrCreateReferenceSpace(xrSession, &spaceCreateInfo, &xrSpace)),
        "Failed to create reference space\n");

    WLOG(INFO, "OpenXR initialized successfully");
	createSwapChains();
}

void updateXRViews(XrView views[2], XrTime predictedDisplayTime) {
    XrViewLocateInfo viewLocateInfo = { XR_TYPE_VIEW_LOCATE_INFO, 
    					.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
    					.displayTime = predictedDisplayTime,
    					.space = xrSpace };

    XrViewState viewState = { XR_TYPE_VIEW_STATE };

    int i;
	for(i=0;i<2;i++) {
        views[i].type = XR_TYPE_VIEW;
        views[i].next = NULL;
    }

    uint32_t viewCountOutput;
    XrResult result = xrLocateViews( xrSession,
    	&viewLocateInfo, &viewState, 2, &viewCountOutput, views);

    if (XR_FAILED(result)) WLOG(ERROR, "xrLocateViews failed!");
}

void xrPoseToMatrix(const XrPosef* pose, mat4x4 matrix) {
    mat4x4_identity(matrix);

    matrix[3][0] = pose->position.x;
    matrix[3][1] = pose->position.y;
    matrix[3][2] = pose->position.z;

    /*convert quaternion to rot mat*/
    float x = pose->orientation.x;
    float y = pose->orientation.y;
    float z = pose->orientation.z;
    float w = pose->orientation.w;

    matrix[0][0] = 1.0f - 2.0f * (y * y + z * z);
    matrix[0][1] = 2.0f * (x * y - z * w);
    matrix[0][2] = 2.0f * (x * z + y * w);

    matrix[1][0] = 2.0f * (x * y + z * w);
    matrix[1][1] = 1.0f - 2.0f * (x * x + z * z);
    matrix[1][2] = 2.0f * (y * z - x * w);

    matrix[2][0] = 2.0f * (x * z - y * w);
    matrix[2][1] = 2.0f * (y * z + x * w);
    matrix[2][2] = 1.0f - 2.0f * (x * x + y * y);
}

void xrFovToProjectionMatrix(const XrFovf* fov, float nearZ, float farZ, mat4x4 matrix) {
    float tanLeft = tanf(fov->angleLeft);
    float tanRight = tanf(fov->angleRight);
    float tanUp = tanf(fov->angleUp);
    float tanDown = tanf(fov->angleDown);

    float width = tanRight - tanLeft;
    float height = tanUp - tanDown;

    matrix[0][0] = 2.0f / width;
    matrix[1][1] = 2.0f / height;
    matrix[2][0] = (tanRight + tanLeft) / width;
    matrix[2][1] = (tanUp + tanDown) / height;
    matrix[2][2] = -(farZ + nearZ) / (farZ - nearZ);
    matrix[2][3] = -1.0f;
    matrix[3][2] = -(2.0f * farZ * nearZ) / (farZ - nearZ);
    matrix[3][3] = 0.0f;
}

bool OEPollXREvents() {
    XrEventDataBuffer eventData = { XR_TYPE_EVENT_DATA_BUFFER };
    XrResult result = xrPollEvent(xrInstance, &eventData);
    while (result == XR_SUCCESS) {
    	if(eventData.type==XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED) {
                XrEventDataSessionStateChanged* sessionStateChangedEvent = 
					(XrEventDataSessionStateChanged*)&eventData;
                xrSessionState = sessionStateChangedEvent->state;

                if (xrSessionState == XR_SESSION_STATE_READY) {
                    XrSessionBeginInfo sessionBeginInfo = { XR_TYPE_SESSION_BEGIN_INFO };
                    sessionBeginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
                    XrResult beginResult = xrBeginSession(xrSession, &sessionBeginInfo);
                    if (XR_FAILED(beginResult)) {
						char buf[1024];
						sprintf(buf, "Failed to begin session with code %d", beginResult);
                        WLOG(ERROR, buf);
                        return false;
                    }
                } else if (xrSessionState == XR_SESSION_STATE_EXITING || 
						xrSessionState == XR_SESSION_STATE_LOSS_PENDING) return false;
        }

        eventData = (XrEventDataBuffer){ XR_TYPE_EVENT_DATA_BUFFER };
        result = xrPollEvent(xrInstance, &eventData);
    }

    if(result != XR_EVENT_UNAVAILABLE) {
        char buf[1024];
		sprintf(buf, "Error polling OpenXR events: %d", result);
		WLOG(ERROR, buf);
        return false;
    }

    return true;
}

/*This currently does not allow for moving the position, but you can look around*/
void updateCamera(XrView view) {
	Camera *cam = OEGetCamera();
	mat4x4 xrView;
    xrPoseToMatrix(&view.pose, xrView);

	mat4x4 camTranslation, out, combTransform;
    mat4x4_translate(camTranslation, cam->position[0], cam->position[1], cam->position[2]);
    
	mat4x4_mul(out, cam->rotation, camTranslation);

    mat4x4_mul(cam->view, xrView, out);

	xrFovToProjectionMatrix(&view.fov, 0.1f, 100.0f, cam->proj);

}

void OERenderXRFrame(RENDFUNC drawCall) {
	OERendererTimerStart();

	XrFrameWaitInfo frameWaitInfo = { XR_TYPE_FRAME_WAIT_INFO };
    XrFrameState frameState = { XR_TYPE_FRAME_STATE };
    WASSERT(!XR_FAILED(xrWaitFrame(xrSession, &frameWaitInfo, &frameState)),
        "Failed to wait for frame");

    WASSERT(!XR_FAILED(xrBeginFrame(xrSession, &(XrFrameBeginInfo){ XR_TYPE_FRAME_BEGIN_INFO })),
        "Failed to begin frame");

    XrView views[2];
    updateXRViews(views, frameState.predictedDisplayTime);

	sg_pass_action pass_action = (sg_pass_action) {
       	.colors[0] = {
           	.load_action = SG_LOADACTION_CLEAR,
       		.clear_value = { 0.0f, 0.3f, 0.5f, 1.0f },
        },
		.depth = {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = 1.0f,
        },
    };

    XrCompositionLayerProjectionView projectionLayerViews[2];

    for (int eye = 0; eye < 2; eye++) {
        uint32_t imageIndex;
        WASSERT(!XR_FAILED(xrAcquireSwapchainImage(xrSwapchains[eye],
						&(XrSwapchainImageAcquireInfo){ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO }, &imageIndex)),
            "Failed to acquire swapchain image");

        WASSERT(!XR_FAILED(xrWaitSwapchainImage(xrSwapchains[eye], 
						&(XrSwapchainImageWaitInfo){ 
							XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO, 
							.timeout = XR_INFINITE_DURATION })),
            "Failed to wait for swapchain image");


		updateCamera(views[eye]);

        sg_begin_pass(&(sg_pass){
				.attachments = passes[eye][imageIndex], 
				.action = pass_action});

        drawCall();

        sg_end_pass();
        sg_commit();

        WASSERT(!XR_FAILED(xrReleaseSwapchainImage(xrSwapchains[eye], 
						&(XrSwapchainImageReleaseInfo){ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO })),
            "Failed to release swapchain image\n");

        projectionLayerViews[eye] = (XrCompositionLayerProjectionView){
            .type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW,
            .pose = views[eye].pose,
            .fov = views[eye].fov,
            .subImage = {
                .swapchain = xrSwapchains[eye],
                .imageRect = { .offset = {0, 0},
					.extent = { (int32_t)swapchain_width,
								(int32_t)swapchain_height }},
                .imageArrayIndex = 0
            }
        };
    }

    XrCompositionLayerProjection layer = {
        .type = XR_TYPE_COMPOSITION_LAYER_PROJECTION,
        .space = xrSpace,
        .viewCount = 2,
        .views = projectionLayerViews
    };

	const XrCompositionLayerBaseHeader * const layers[] = {
    	(const XrCompositionLayerBaseHeader*)&layer
	};
    XrFrameEndInfo frameEndInfo = { XR_TYPE_FRAME_END_INFO };
    frameEndInfo.displayTime = frameState.predictedDisplayTime;
    frameEndInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    frameEndInfo.layerCount = 1;
    frameEndInfo.layers = layers;

    WASSERT(!XR_FAILED(xrEndFrame(xrSession, &frameEndInfo)),
        "Failed to end frame\n");

    OERendererTimerEnd();
}
