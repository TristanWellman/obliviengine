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

## Compile OE

```
$ git clone --recursive https://github.com/TristanWellman/obliviengine
$ cd obliviengine
$ make
```

This should build the library that you can include in your project.

Copy the lib (and dlls), and include folders to your project


## Sample Cube

---

sample.c
```c
#include <OE/OE.h>

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

### Compile

``-Iinclude`` and ``-Llib`` can be replaced with whatever folders you are using.

```
gcc -g -Iinclude sample.c -o sample -Llib -lOE -lSDL2 -lgdi32 -lopengl32 -lm -ldl -lpthread
```

## OpenXR-SDK

---

* To use VR with obliviengine simply install the OpenXR-SDK mingw package through msys2

```
$ pacman -S mingw-w64-x86_64-openxr-sdk
```

Program setup:

```c
#include <OE/OE.h>
#include <OE/openxr/OEOpenxr.h>

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


