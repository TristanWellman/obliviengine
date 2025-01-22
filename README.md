# Obliviengine

My dumb little renderer.

--- 

You need to put these files into ``include``

* stb_image.h
* nuklear.h
* SDL2/

## Sample Cube

---

```c
#include "engine/renderer.h"
#include "engine/openxr/renderer_openxr.h"

void draw() {
	drawObject(getObjectFromName("Cube"));
}

void event() {
	SDL_Event event = getEvent();
	float camSpeed = 5.0f*getFrameTime();
	if(isKeyPressed()) {
		Vec3 pos = getCamPos();
		switch(getKeySym()) {
			case SDLK_w: moveCam(FRONT, camSpeed); break;
			case SDLK_s: moveCam(BACKWARD, camSpeed); break;
			case SDLK_a: moveCam(LEFT, camSpeed); break;
			case SDLK_d: moveCam(RIGHT, camSpeed); break;
			case SDLK_SPACE: moveCam(UP, camSpeed); break;
			case SDLK_RSHIFT:
			case SDLK_LSHIFT: moveCam(DOWN, camSpeed); break;
		};
	}	
}

int main(int argc, char **argv) {
	
	initRenderer(1280, 720, "game", PERSPECTIVE);
	enableDebugInfo();
	
	while(rendererIsRunning()) {
		pollEvents((EVENTFUNC)event);
		renderXRFrame((RENDFUNC)draw);
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
}
void event() {
}

int main(int argc, char **argv) {
	
	initRenderer(1280, 720, "game", PERSPECTIVE);
	initOpenXR();
	enableDebugInfo();
	
	while(rendererIsRunning()) {
		pollEvents((EVENTFUNC)event);
		pollXREvents();
		renderXRFrame((RENDFUNC)draw);
	}
	return 0;
}
```

--- 

<div align="center">
  <br />
  <p>
    <a><img src="https://github.com/TristanWellman/obliviengine/blob/main/voxelTrace.png" width="800" alt="wellang" /></a>
  </p>
  <br />
</div>
