#load "../OE.jai";

/* We need to do this since we aren't implementing the foreign functions from outside OE, 
 * meaning the compiler won't auto-link the other libs.*/
#run {
	set_build_options_dc(.{append_linker_arguments=string.[
		"-lm", "-lminizip", "-lpthread", "-lc++",
		"-L../../lib/mac", "../../lib/mac/libminizip.a",
		"../../lib/mac/libSDL2.a", "-lcimgui", "-lassimp",
		"-llua", "-framework", "Cocoa", "-framework", "OpenGL", 
		"-framework", "Cocoa", "-framework", "IOKit",
		"-framework", "CoreVideo", "-framework", "AudioToolbox", 
		"-framework", "CoreHaptics", "-framework", "GameController", 
		"-framework", "Metal", "-framework", "QuartzCore",
		"-framework", "ForceFeedback", "-framework", "Carbon", 
		"-framework", "CoreFoundation", "-framework", "CoreAudio"
	]});
}

OE :: #library,no_dll "../../lib/mac/libOE";

render :: () #c_call {
	OEDrawObject(OEGetObjectFromName("OECube"));

	batch := OEGetInstanceBatchFromName("OECube");
	OEPushInstanceBatchData(batch, cast(*vec3)(float32.[-2.0, 2.0, 2.0]).data);
	OEPushInstanceBatchData(batch, cast(*vec3)(float32.[2.0, 3.0, 2.2]).data);
	OEPushInstanceBatchData(batch, cast(*vec3)(float32.[-3.0, 1.0, -3.0]).data);
	OEPushInstanceBatchData(batch, cast(*vec3)(float32.[-5.0, 3.0, 1.0]).data);
	OEDrawInstanceBatch(batch);
}

event :: () #c_call {
	event := OEGetEvent();
	camSpeed : float = 5.0*OEGetFrameTime();
	if OEIsKeyPressed() {
		pos := OEGetCamPos();
		key := OEGetKeySym();
		if key == cast(s32)SDLK_w then OEMoveCam(face.FRONT, camSpeed);
		else if key == cast(s32)SDLK_s then OEMoveCam(face.BACKWARD, camSpeed);
		else if key == cast(s32)SDLK_a then OEMoveCam(face.LEFT, camSpeed);
		else if key == cast(s32)SDLK_d then OEMoveCam(face.RIGHT, camSpeed);
		else if key == cast(s32)SDLK_SPACE then OEMoveCam(face.UP, camSpeed);
		else if key == cast(s32)SDLK_LSHIFT then OEMoveCam(face.DOWN, camSpeed);
	}
}

main :: () {
	OEInitRenderer(1280, 720, "cube", CamType.PERSPECTIVE);
	defer OECleanup();

	OEEnableDebugInfo();
	OEEnableFXAA();
	OEEnableSSGI(24, 8);
	OEEnableBloom(0.8, 0.5);

	OESetObjectPosition("OECube", cast(*vec3)(float32.[0.0, -1.0, 0.0]).data);
	OECreateInstanceBatch(OEGetObjectFromName("OECube"));

	red := Color.{255.0, 118.0, 118.0, 255.0};	
	red /= 255.0;
	OEAddLight("TestLight", cast(*vec3)(float32.[3.0, 3.0, 3.0]).data, red);

	while OERendererIsRunning() {
		OEPollEvents(event);
		OERenderFrame(render, null, null);
	}
}
