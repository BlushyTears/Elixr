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

#include "ElixrUtil.h"

CameraController camera;
ParticleEmitter pe;

int main ()
{
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	std::string KnightModel = "robot.glb";
	InitWindow(800, 600, "Hello Raylib");
	SearchAndSetResourceDir("resources");
	Model model = LoadModel(KnightModel.c_str());

	std::vector<float> vertexPositions;
	const int modelCount = model.meshCount;
	float modelScale = 1.5f;

	SetParticlePositions(modelCount, modelScale, model, vertexPositions);

	float pitch = 0.0f;
	float yaw = 0.0f;
	float roll = 0.0f;

	const int ParticlesToSpawn = vertexPositions.size() / 3;
	pe.SpawnParticles(ParticlesToSpawn, true, vertexPositions);

	SingleForceProperty upforceProperty;
	WaveProperty waveProperty;
	RandomColorChangeProperty ccp;

	ccp.minColor.x = 200;
	ccp.minColor.y = 0;
	ccp.minColor.z = 0;

	ccp.maxColor.x = 255;
	ccp.maxColor.y = 50;
	ccp.maxColor.z = 50;

	RandomColorChangeProperty baseColorRandom;

	baseColorRandom.minColor.x = 0;
	baseColorRandom.minColor.y = 0;
	baseColorRandom.minColor.z = 0;

	baseColorRandom.maxColor.x = 255;
	baseColorRandom.maxColor.y = 60;
	baseColorRandom.maxColor.z = 80;

	ColorStaticProperty csp;
	FadeoutColorProperty fcp;
	FlockbehaviorProperty fbp;
	VortexProperty vp;
	PointAttractorProperty pap;

	for (int i = 0; i < ParticlesToSpawn; i++)
	{
		pe.particles[i]->SetLifeTime(GetRandomFloatRange(0.05, 0.15));
		pe.particles[i]->AttatchProperty(&baseColorRandom);

		if (i % 15 == 0)
		{
			pe.particles[i]->scale = modelScale * 10;
			pe.particles[i]->SetLifeTime(GetRandomFloatRange(0.6, 1.2));
			pe.particles[i]->AttatchProperty(&ccp);
			pe.particles[i]->AttatchProperty(&waveProperty);
			pe.particles[i]->AttatchProperty(&fcp);

		}
		if (i % 4 == 0)
		{
			pe.particles[i]->AttatchProperty(&upforceProperty);
			pe.particles[i]->AttatchProperty(&vp);
		}
		if (i % 64 == 0)
		{
			pe.particles[i]->AttatchProperty(&csp);
			pe.particles[i]->AttatchProperty(&fcp);
		}

		pe.particles[i]->AttatchProperty(&pap);
		//pe.particles[i]->AttatchProperty(&vp);
		//pe.particles[i]->AttatchProperty(&pap);
	}

	SetTargetFPS(30);

	Shader shader = LoadShader("../lighting.vs", "../lighting.fs");

	shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

	int ambientLoc = GetShaderLocation(shader, "ambient");

	Light lights[3] = { 0 };
	lights[0] = CreateLight(LIGHT_POINT, Vector3{ -8, 1, -8 }, Vector3Zero(), BLUE, shader);
	lights[1] = CreateLight(LIGHT_POINT, Vector3{ 3, 1, 6 }, Vector3Zero(), RED, shader);
	lights[2] = CreateLight(LIGHT_POINT, Vector3{ 8, 1, -8 }, Vector3Zero(), YELLOW, shader);

	Model plane = LoadModelFromMesh(GenMeshPlane(20.0f, 20.0f, 1, 1));
	plane.materials[0].shader = shader;

	for (int i = 0; i < model.materialCount; i++)
	{
		model.materials[i].shader = shader;
	}

	int animCount = 0;
	ModelAnimation* anims = LoadModelAnimations(KnightModel.c_str(), &animCount);

	while (!WindowShouldClose())
	{
		PlayIdleAnimation(model, anims, animCount);
		SetParticlePositions(modelCount, modelScale, model, vertexPositions);

		for (auto* p : pe.particles)
		{
			int v = p->vertexIdx * 3;
			if (v + 2 < (int)vertexPositions.size())
			{
				Vector3 animPos = {
					vertexPositions[v],
					vertexPositions[v + 1],
					vertexPositions[v + 2]
				};
				p->startingPos = animPos;

				if (p->properties.empty())
				{
					p->position = animPos;
				}
			}
		}

		if (IsKeyPressed(KEY_B)) { lights[0].enabled = !lights[0].enabled; }
		if (IsKeyPressed(KEY_R)) { lights[1].enabled = !lights[1].enabled; }
		if (IsKeyPressed(KEY_Y)) { lights[2].enabled = !lights[2].enabled; }

		// Update light values (actually, only enable/disable them)
		for (int i = 0; i < 3; i++) 
			UpdateLightValues(shader, lights[i]);

		UpdateCamera(&camera.cam, CAMERA_FREE);

		// needed for specular lighting
		float cameraPos[3] = { camera.cam.position.x, camera.cam.position.y, camera.cam.position.z };
		SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

		DisableCursor();

		Vector3 angle = { DEG2RAD * pitch, DEG2RAD * yaw, DEG2RAD * roll };

		BeginDrawing();
		ClearBackground(BLACK);

		BeginMode3D(camera.cam);

		// Exists for debugging purposes
		//DrawModelSkeleton(model.skeleton, model.currentPose, 1.0f, GREEN);

		//for (int i = 0; i < 2; i++)
		//{
		//	if (lights[i].enabled) DrawSphereEx(lights[i].position, 0.2f, 8, 8, lights[i].color);
		//	else DrawSphereWires(lights[i].position, 0.8f, 8, 8, ColorAlpha(lights[i].color, 0.3f));
		//}

		//DrawModel(model, { 0, 0, 0 }, 1.0f, BLACK);
		DrawModel(plane, { 0, -1, 0 }, 1.0f, GRAY);

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
