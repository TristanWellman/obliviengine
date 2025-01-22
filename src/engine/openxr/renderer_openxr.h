/*Copyright (c) 2025 Tristan Wellman
 *
 * Obliviengine OpenXR currently only works for windows
 *
 * */
#ifndef RENDERER_OPENXR_H
#define RENDERER_OPENXR_H

#include "../renderer.h"

#define MAX_SWAPCHAIN_IMAGES 4
#define NUM_EYES 2

/*You are meant to run this after runing initRenderer()*/
void initOpenXR();
bool pollXREvents();
void renderXRFrame(RENDFUNC drawCall);

#endif
