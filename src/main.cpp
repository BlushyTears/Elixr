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
	RandomColorChangeProperty ccp;
	ColorStaticProperty csp;
	FadeoutColorProperty fcp;
	FlockbehaviorProperty fbp;
	VortexProperty vp;
	PointAttractorProperty pap;

	for (int i = 0; i < ParticlesToSpawn; i++)
	{
		pe.particles[i]->SetLifeTime(GetRandomFloatRange(1.0, 5.0));
		//pe.particles[i]->AttatchProperty(&upforceProperty);
		//pe.particles[i]->AttatchProperty(&waveProperty);
		//pe.particles[i]->AttatchProperty(&ccp);
		pe.particles[i]->AttatchProperty(&csp);
		//pe.particles[i]->AttatchProperty(&fcp);
		pe.particles[i]->AttatchProperty(&fbp);
		pe.particles[i]->AttatchProperty(&vp);
		//pe.particles[i]->AttatchProperty(&pap);
	}

	Shader shader = LoadShader("../lighting.vs", "../lighting.fs");

	shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

	int ambientLoc = GetShaderLocation(shader, "ambient");

	Light lights[2] = { 0 };
	lights[0] = CreateLight(LIGHT_POINT, Vector3{ -2, 1, -2 }, Vector3Zero(), BLUE, shader);
	lights[1] = CreateLight(LIGHT_POINT, Vector3{ 2, 1, 2 }, Vector3Zero(), RED, shader);

	Model plane = LoadModelFromMesh(GenMeshPlane(20.0f, 20.0f, 1, 1));
	plane.materials[0].shader = shader;

	for (int i = 0; i < model.materialCount; i++)
	{
		model.materials[i].shader = shader;
	}

	while (!WindowShouldClose())
	{
		if (IsKeyPressed(KEY_B)) { lights[0].enabled = !lights[0].enabled; }
		if (IsKeyPressed(KEY_R)) { lights[1].enabled = !lights[1].enabled; }

		// Update light values (actually, only enable/disable them)
		for (int i = 0; i < 2; i++) 
			UpdateLightValues(shader, lights[i]);

		UpdateCamera(&camera.cam, CAMERA_FREE);

		// needed for specular lighting
		float cameraPos[3] = { camera.cam.position.x, camera.cam.position.y, camera.cam.position.z };
		SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

		DisableCursor();

		Vector3 angle = { DEG2RAD * pitch, DEG2RAD * yaw, DEG2RAD * roll };

		model.transform = MatrixMultiply(MatrixRotateXYZ(angle), MatrixScale(3, 3, 3));

		BeginDrawing();
		ClearBackground(BLACK);

		BeginMode3D(camera.cam);

		for (int i = 0; i < 2; i++)
		{
			if (lights[i].enabled) DrawSphereEx(lights[i].position, 0.2f, 8, 8, lights[i].color);
			else DrawSphereWires(lights[i].position, 0.8f, 8, 8, ColorAlpha(lights[i].color, 0.3f));
		}

		DrawModel(model, { 0, 2, 0 }, 1.0f, GRAY);
		DrawModel(plane, { 0, -5, 0 }, 1.0f, GRAY);

		pe.ManageParticles();
		pe.RenderAllParticles();

		DrawGrid(10, 10.0f);
		EndMode3D();

		EndDrawing();
	}

	UnloadShader(shader);

	CloseWindow();
	return 0;
}
