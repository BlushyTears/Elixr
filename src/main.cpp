#include <iostream>

#include "raylib.h"
#include "raymath.h"
#include "resource_dir.h"

#include "ModelLoading.h"
#include "CameraController.h"
#include "ParticlePrimitives.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            100
#endif

#include "ElixrUtil.h"
#include "SceneStates.h"

// Idea: Start att menu where you can create new scenes
// From menu state you are taken to particle state which will be an empty scene
// From particle scene you can open blueprint state which will display all blueprints as a grid
// Left joycon pans and right is the cursor
// Right is for controlling a specific blueprint, and left is for selecting a new
// When in a particle scene you freecam fly with left joycon and rotate with right joycon
// In menu state only right joycon is used for cursor

enum State 
{
	MenuState,
	BlueprintState,
	ParticleState
};

SampleScene ss;

int main()
{
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	SetTargetFPS(30);

	InitWindow(800, 600, "Hello Raylib");
	SearchAndSetResourceDir("resources");

	ss.SetupData();
	ss.SetupMainScene();

	State msms;
	msms = MenuState;

	while (!WindowShouldClose())
	{
		if (IsKeyPressed(KEY_ONE)) { msms = MenuState; }
		if (IsKeyPressed(KEY_TWO)) { msms = BlueprintState; }
		if (IsKeyPressed(KEY_THREE)) { msms = ParticleState; }

		switch(msms)
		{
		case MenuState:
			ss.MenuScene();
			break;
		case BlueprintState:
			break;
		case ParticleState:
			ss.DisplaySampleScene();
			break;
		}
	}

	CloseWindow();
	return 0;
}
