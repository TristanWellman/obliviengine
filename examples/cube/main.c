/*Copyright (c) 2025 Tristan Wellman
 *
 * Cube example 
 *
 * */
#include <OE/OE.h>

void draw() {
	OEDrawObject(OEGetObjectFromName("OECube"));
}

void event() {
	// OE uses SDL for key handling, this is why we use SDL_Event.
	SDL_Event event = OEGetEvent();
	// Create a camera speed based on delta-time
	float camSpeed = 5.0f*OEGetFrameTime();
	
	if(OEIsKeyPressed()) {
		// get the camera position and move it based on the key press.
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
	
	// Initialize OE for a 1280x720 window size and give the camera a PERSPECTIVE view.
	OEInitRenderer(1280, 720, "cube", PERSPECTIVE);
	// enable debug info (fps, frametime, camera position)
	OEEnableDebugInfo();

	// Set the position of the cube to -3,0,0
	OESetObjectPosition("OECube", (vec3){-3.0f, 0.0f, 0.0f});

	// Add another light to the scene
	Color c = (Color){77.0f, 106.0f, 148.0f, 255.0f};
	OEAddLight("Test", (vec3){2.0f, 2.0f, 2.0f}, RGBA255TORGBA1(c));

	// Renderer loop
	while(OERendererIsRunning()) {
		// Get key events
		OEPollEvents((EVENTFUNC)event);
		// Render the cube
		OERenderFrame((RENDFUNC)draw, NULL);
	}
	
	return 0;
}
