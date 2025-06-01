/*Copyright (c) 2025 Tristan Wellman
 *
 * This is a super simple example/testing program
 *
 * */
#include <OE/OE.h>
#include <time.h>
//#include "engine/openxr/OEOpenxr.h"

void draw() {
	//OEDrawObject(OEGetObjectFromName("Icosphere"));
	OEDrawObjectTex(OEGetObjectFromName("Cube"), OE_TEXPOS, getTexture("test"));
	OEDrawObjectTex(OEGetObjectFromName("OECube"), OE_TEXPOS, getTexture("test"));
	OEDrawObject(OEGetObjectFromName("OEPlane"));

	int i;
	for(i=0;i<15;i++) {
		char buf[256];
		snprintf(buf, sizeof(buf), "%s%d", "OECube", i);
		OEDrawObject(OEGetObjectFromName(buf));
	}
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

void colorTest() {
	/*This is so you can convert a 255 color value to 1*/
	Color test = {100.0f, 181.0f, 72.0f, 255.0f};
	Color res = RGBA255TORGBA1(test);
	printf("(%f, %f, %f, %f)\n", res.r, res.g, res.b, res.a);
}

void meshTest() {
	OEMesh mesh, sphere;
	OEParseObj("Cube", "./assets/models/cube.obj", &mesh);
	sg_shader defShader = OEGetDefCubeShader();
	OECreateObjectFromMesh(&mesh, (vec3){0.0f,0.0f,0.0f});
	/*OEParseObj("assets/models/sphere.obj", &sphere);
	OECreateObjectFromMesh(&sphere, (vec3){3.0f,0.0f,0.0f});*/

}

void makeCubes() {
	int i;
	srand(pow(clock(),10));
	for(i=0;i<15;i++) {
		char buf[256];
		snprintf(buf, sizeof(buf), "%s%d", "OECube", i);
		sg_shader s = OEGetDefCubeShader();
		OECreateObjectEx(buf, (vec3){(rand()%20)-10,(rand()%15),(rand()%20)-10},
				OEGetCubeVertDesc(),OEGetCubeIndDesc(), s, OEGetDefaultPipe(s,buf));
	}

}

int main(int argc, char **argv) {
	
	OEInitRenderer(1280, 720, "game", PERSPECTIVE);
	//initOpenXR();
	OEEnableDebugInfo();
	meshTest();
	//OESetDefaultShader(OEGetRayTracedShader());

	OEEnableBloom(0.8, 0.5);
	OEEnableFXAA();
	OEEnableSSAO();

	makeCubes();

	OESetObjectPosition("OEPlane", (vec3){0.0f, -1.0f, 0.0f});
	OESetObjectPosition("OECube", (vec3){-3.0f, 0.0f, 0.0f});

	Color light1 = (Color){255.0f, 50.0f, 50.0f, 255.0f};
	Color light2 = (Color){50.0f, 50.0f, 255.0f, 255.0f};

	OEAddLight("MainLight", (vec3){3.0f, 3.0f, 3.0f}, RGBA255TORGBA1(light2));
	OEAddLight("FillLight", (vec3){-3.0f, 3.0f, -3.0f}, RGBA255TORGBA1(light1));

	addTexture("test", "assets/uvtest.jpg");

	while(OERendererIsRunning()) {
		OEPollEvents((EVENTFUNC)event);
		//OEPollXREvents();
		//OERenderXRFrame((RENDFUNC)draw);
		OERenderFrame((RENDFUNC)draw);
	}
	
	return 0;
}
