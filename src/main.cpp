/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/

#include <iostream>

#include "raylib.h"
#include "raymath.h"
#include "resource_dir.h"

#include "ModelLoading.h"
#include "CameraController.h"
#include "ParticlePrimitives.h"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"


#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            100
#endif

CameraController camera;
ParticleEmitter pe;

int main ()
{
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	InitWindow(800, 600, "Hello Raylib");
	SearchAndSetResourceDir("resources");
	Model model = LoadModel("Knight.glb");

	float pitch = 0.0f;
	float yaw = 0.0f;
	float roll = 0.0f;

	const int ParticlesToSpawn = 1000;
	pe.SpawnParticles(ParticlesToSpawn, false);

	SingleForceProperty upforceProperty;
	WaveProperty waveProperty;
	ColorChangeProperty ccp;
	ColorStaticProperty csp;
	FadeoutColorProperty fcp;
	FlockbehaviorProperty fbp;
	VortexProperty vp;

	for (int i = 0; i < ParticlesToSpawn; i++)
	{
		pe.particles[i]->SetLifeTime(GetRandomFloatRange(1.0, 5.0));
		//pe.particles[i]->AttatchProperty(&upforceProperty);
		//pe.particles[i]->AttatchProperty(&waveProperty);
		//pe.particles[i]->AttatchProperty(&ccp);
		pe.particles[i]->AttatchProperty(&csp);
		pe.particles[i]->AttatchProperty(&fcp);
		pe.particles[i]->AttatchProperty(&fbp);
		pe.particles[i]->AttatchProperty(&vp);
	}

	Shader shader = LoadShader(TextFormat("resources/shaders/glsl%i/lighting.vs", GLSL_VERSION),
		TextFormat("resources/shaders/glsl%i/lighting.fs", GLSL_VERSION));

	shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

	int ambientLoc = GetShaderLocation(shader, "ambient");

	float data[4] = {0.1f, 0.1f, 0.1f, 1.0f};
	SetShaderValue(shader, ambientLoc, data, SHADER_UNIFORM_VEC4);

	Light lights[MAX_LIGHTS] = { 0 };
	lights[0] = CreateLight(LIGHT_POINT, (Vector3) ( -2, 1, -2 ), Vector3Zero(), YELLOW, shader);
	lights[1] = CreateLight(LIGHT_POINT, (Vector3) ( 2, 1, 2 ), Vector3Zero(), RED, shader);
	lights[2] = CreateLight(LIGHT_POINT, (Vector3) ( -2, 1, 2 ), Vector3Zero(), GREEN, shader);
	lights[3] = CreateLight(LIGHT_POINT, (Vector3) ( 2, 1, -2 ), Vector3Zero(), BLUE, shader);

	while (!WindowShouldClose())
	{
		Vector3 angle = { DEG2RAD * pitch, DEG2RAD * yaw, DEG2RAD * roll };

		model.transform = MatrixRotateXYZ(angle);
		DrawPlane(Vector3Zero(), (Vector2) ( 10.0, 10.0 ), WHITE);
		DrawCube(Vector3Zero(), 2.0, 4.0, 2.0, WHITE);
		camera.Freecam();

		BeginDrawing();
		ClearBackground(BLACK);

		BeginMode3D(camera.cam);

		//BeginShaderMode(shader);
		//	DrawPlane(Vector3Zero(), (Vector2) ( 10.0, 10.0 ), WHITE);
		//	DrawCube(Vector3Zero(), 2.0, 4.0, 2.0, WHITE);
		//EndShaderMode();

		pe.ManageParticles();
		pe.RenderAllParticles(&fbp);

		DrawGrid(10, 10.0f);
		EndMode3D();

		EndDrawing();
	}

	CloseWindow();
	return 0;
}
