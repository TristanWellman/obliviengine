/*Copyright (c) 2025 Tristan Wellman
 *
 * Obliviengine OpenXR currently only works for windows
 *
 * */
#ifndef OEOPENXR_H
#define OEOPENXR_H

#include "../OE.h"

#define MAX_SWAPCHAIN_IMAGES 4
#define NUM_EYES 2

/*You are meant to run this after runing initRenderer()*/
void OEInitOpenXR();
bool OEPollXREvents();
void OERenderXRFrame(RENDFUNC drawCall);

#endif
