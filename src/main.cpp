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

CameraController camera;

ParticleManager pm;

void SpawnParticle()
{
	int x = GetRandomValue(-10, 10);
	int y = GetRandomValue(-1, 1);
	int z = GetRandomValue(-10, 10);

	pm.AddCubeParticle({ (float)x, (float)y, (float)z }, Color(255, 0, 0, 255));
}

void SpawnParticles()
{
	for (int i = 0; i < 3; i++)
	{
		SpawnParticle();
	}
}

void ApplyForce(BaseParticle* bp)
{
	Vector3 force(0, 0.5f, 0);

	bp->force += force;
	bp->acceleration += force;
	bp->position += bp->acceleration * GetFrameTime();
}

void ManageParticles()
{
	int particlesToSpawn = 0;

	std::erase_if(pm.particles, [&particlesToSpawn](auto& particle)
	{
		if (!particle->lifeTime)
			return false;

		particle->lifeTime -= GetFrameTime();

		if (particle->lifeTime <= 0)
		{
			++particlesToSpawn;
			delete particle;
			return true;
		}

		ApplyForce(particle);
		return false;
	});

	for (int i = 0; i < particlesToSpawn; ++i)
	{
		SpawnParticle();
	}
}

int main ()
{
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	InitWindow(800, 600, "Hello Raylib");
	SearchAndSetResourceDir("resources");
	Model model = LoadModel("Knight.glb");

	float pitch = 0.0f;
	float yaw = 0.0f;
	float roll = 0.0f;

	SpawnParticles();

	while (!WindowShouldClose())
	{
		Vector3 angle = { DEG2RAD * pitch, DEG2RAD * yaw, DEG2RAD * roll };

		model.transform = MatrixRotateXYZ(angle);

		camera.Freecam();

		BeginDrawing();
		ClearBackground(BLACK);

		BeginMode3D(camera.cam);

		ManageParticles();

		pm.RenderAllParticles();
		//DrawText("Hello Raylib", 200, 200, 20, WHITE);

		DrawGrid(10, 10.0f);
		EndMode3D();

		EndDrawing();
	}

	CloseWindow();
	return 0;
}
