# Obliviengine

My dumb little renderer.

--- 

<div align="center">
  <br />
  <p>
    <a><img src="https://github.com/TristanWellman/obliviengine/blob/main/voxelTrace.png" width="800" alt="wellang" /></a>
  </p>
  <br />
</div>

--- 

You need to put these files into ``include``

* stb_image.h
* nuklear.h
* SDL2/

## Sample Cube

---

```c
#include "engine/OE.h"

void draw() {
	OEDrawObject(OEGetObjectFromName("OECube"));
}

void event() {
	SDL_Event event = OEGetEvent();
	float camSpeed = 5.0f*OEGetFrameTime();
	if(OEIsKeyPressed()) {
		Vec3 pos = OEGetCamPos();
		switch(OEGetKeySym()) {
			case SDLK_w: OEMoveCam(FRONT, camSpeed); break;
			case SDLK_s: OEMoveCam(BACKWARD, camSpeed); break;
			case SDLK_a: OEMoveCam(LEFT, camSpeed); break;
			case SDLK_d: OEMoveCam(RIGHT, camSpeed); break;
			case SDLK_SPACE: OEMoveCam(UP, camSpeed); break;
			case SDLK_RSHIFT:
			case SDLK_LSHIFT: OEMoveCam(DOWN, camSpeed); break;
		};
	}	
}

int main(int argc, char **argv) {
	
	OEInitRenderer(1280, 720, "game", PERSPECTIVE);
	OEEnableDebugInfo();
	
	while(OERendererIsRunning()) {
		OEPollEvents((EVENTFUNC)event);
		OERenderFrame((RENDFUNC)draw);
	}
	
	return 0;
}
```

## OpenXR-SDK

---

* To use VR with obliviengine simply install the OpenXR-SDK mingw package through msys2

```
$ pacman -S mingw-w64-x86_64-openxr-sdk
```

Program setup:

```c
#include "engine/renderer.h"
#include "engine/openxr/renderer_openxr.h"

void draw() {
	OEDrawObject(OEGetObjectFromName("OECube"));
}

void event() {
	SDL_Event event = OEGetEvent();
	float camSpeed = 5.0f*OEGetFrameTime();
	if(OEIsKeyPressed()) {
		Vec3 pos = OEGetCamPos();
		switch(OEGetKeySym()) {
			case SDLK_w: OEMoveCam(FRONT, camSpeed); break;
			case SDLK_s: OEMoveCam(BACKWARD, camSpeed); break;
			case SDLK_a: OEMoveCam(LEFT, camSpeed); break;
			case SDLK_d: OEMoveCam(RIGHT, camSpeed); break;
			case SDLK_SPACE: OEMoveCam(UP, camSpeed); break;
			case SDLK_RSHIFT:
			case SDLK_LSHIFT: OEMoveCam(DOWN, camSpeed); break;
		};
	}	
}

int main(int argc, char **argv) {
	
	OEInitRenderer(1280, 720, "game", PERSPECTIVE);
    OEInitOpenXR();
	OEEnableDebugInfo();
	
	while(OERendererIsRunning()) {
		OEPollEvents((EVENTFUNC)event);
        OEPollXREvents();
		OERenderXRFrame((RENDFUNC)draw);
	}
	
	return 0;
}
```


