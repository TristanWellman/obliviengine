#include <stdio.h>
#include <time.h>
#include "engine/util.h"
#include "engine/renderer.h"
#include "graphics.h"
#include "world.h"

void draw() {
	drawWorld();
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
	initRenderer(1280, 720, "game");
	enableDebugInfo();
	initScreenQ();
	initWorld();
	while(rendererIsRunning()) {
		pollEvents((EVENTFUNC)event);
		renderFrame((RENDFUNC)draw);
	}
}
