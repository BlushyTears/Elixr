#pragma once

#include <iostream>
#include <string>

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

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

CameraController camera;
ParticleEmitter pe;

struct EmitterBlueprint
{
	Vector2 position;
	// is mouse/joycon holding this, and if so we wanna be able to drag it
	bool isHeld = false;
	ParticleEmitter* emitterReference = nullptr;

};

struct PropertyBlueprint
{
	
};

enum State
{
	MenuState,
	SelectParticleState,
	BuildParticleState,
	BlueprintState,
	DisplayParticleState
};

struct SampleScene
{
	Model model;
	std::string KnightModel;
	std::vector<float> vertexPositions;
	int modelCount;
	float modelScale;

	float pitch = 0.0f;
	float yaw = 0.0f;
	float roll = 0.0f;
	int ParticlesToSpawn;

	int animCount;

	SingleForceProperty upforceProperty;
	WaveProperty waveProperty;
	RandomColorChangeProperty ccp;
	RandomColorChangeProperty baseColorRandom;
	ColorStaticProperty csp;
	FadeoutColorProperty fcp;
	FlockbehaviorProperty fbp;
	VortexProperty vp;
	PointAttractorProperty pap;

	Shader shader;
	Light lights[3];
	Model plane;

	ModelAnimation* anims = nullptr;

	void SetupData()
	{
		KnightModel = "robot.glb";
		model = LoadModel(KnightModel.c_str());

		modelCount = model.meshCount;
		modelScale = 1.5f;

		SetParticlePositions(modelCount, modelScale, model, vertexPositions);

		ParticlesToSpawn = vertexPositions.size() / 3;
		pe.SpawnParticles(ParticlesToSpawn, true, vertexPositions);

		ccp.minColor.x = 200;
		ccp.minColor.y = 0;
		ccp.minColor.z = 0;

		ccp.maxColor.x = 255;
		ccp.maxColor.y = 50;
		ccp.maxColor.z = 50;

		baseColorRandom.minColor.x = 0;
		baseColorRandom.minColor.y = 0;
		baseColorRandom.minColor.z = 0;

		baseColorRandom.maxColor.x = 255;
		baseColorRandom.maxColor.y = 60;
		baseColorRandom.maxColor.z = 80;

		for (int i = 0; i < ParticlesToSpawn; i++)
		{
			pe.particles[i]->SetLifeTime(GetRandomFloatRange(0.35, 0.47));
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

			//pe.particles[i]->AttatchProperty(&pap);
			//pe.particles[i]->AttatchProperty(&vp);
			//pe.particles[i]->AttatchProperty(&pap);
		}

		shader = LoadShader("../lighting.vs", "../lighting.fs");

		shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

		int ambientLoc = GetShaderLocation(shader, "ambient");

		lights[0] = CreateLight(LIGHT_POINT, Vector3{ -8, 1, -8 }, Vector3Zero(), BLUE, shader);
		lights[1] = CreateLight(LIGHT_POINT, Vector3{ 3, 1, 6 }, Vector3Zero(), RED, shader);
		lights[2] = CreateLight(LIGHT_POINT, Vector3{ 8, 1, -8 }, Vector3Zero(), YELLOW, shader);

		plane = LoadModelFromMesh(GenMeshPlane(20.0f, 20.0f, 1, 1));
		plane.materials[0].shader = shader;

		for (int i = 0; i < model.materialCount; i++)
		{
			model.materials[i].shader = shader;
		}

		animCount = 0;
		anims = LoadModelAnimations(KnightModel.c_str(), &animCount);
	}

	void DisplaySampleScene(State& msms)
	{
		ClearBackground(BLACK);

		PlayIdleAnimation(model, anims, animCount);
		SetParticlePositions(modelCount, modelScale, model, vertexPositions);

		int gamepad = 0;

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

		//for (int i = 0; i < 4; i++) {
		//	if (IsGamepadAvailable(i)) {
		//		for (int button = 0; button < 32; button++) {
		//			if (IsGamepadButtonPressed(i, button)) {
		//				printf("Slot %d (%s) -> Button: %d\n", i, GetGamepadName(i), button);
		//				fflush(stdout);
		//			}
		//		}

		//		for (int axis = 0; axis < 4; axis++) {
		//			float val = GetGamepadAxisMovement(i, axis);
		//			if (val > 0.3f || val < -0.3f) {
		//				printf("Slot %d -> Stick Axis %d: %.2f\n", i, axis, val);
		//				fflush(stdout);
		//			}
		//		}
		//	}
		//}

		if (IsKeyPressed(KEY_B)) { lights[0].enabled = !lights[0].enabled; }
		if (IsKeyPressed(KEY_R)) { lights[1].enabled = !lights[1].enabled; }
		if (IsKeyPressed(KEY_Y)) { lights[2].enabled = !lights[2].enabled; }

		// Update light values (actually, only enable/disable them)
		for (int i = 0; i < 3; i++)
			UpdateLightValues(shader, lights[i]);

		// needed for specular lighting
		float cameraPos[3] = { camera.cam.position.x, camera.cam.position.y, camera.cam.position.z };
		SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

		BeginDrawing();

		BeginMode3D(camera.cam);

		// Exists for debugging purposes
		//DrawModelSkeleton(model.skeleton, model.currentPose, 1.0f, GREEN);

		// draw debug lights
		//for (int i = 0; i < 2; i++)
		//{
		//	if (lights[i].enabled) DrawSphereEx(lights[i].position, 0.2f, 8, 8, lights[i].color);
		//	else DrawSphereWires(lights[i].position, 0.8f, 8, 8, ColorAlpha(lights[i].color, 0.3f));
		//}

		//DrawModel(model, { 0, 0, 0 }, 1.55f, BLACK);
		DrawModel(plane, { 0, -1, 0 }, 1.0f, GRAY);

		pe.ManageParticles();
		pe.RenderAllParticles();

		DrawGrid(10, 10.0f);
		EndMode3D();

		if (IsMouseButtonDown(MouseButton::MOUSE_BUTTON_RIGHT))
		{
			HideCursor();
			UpdateCamera(&camera.cam, CAMERA_FREE);
		}
		else
			ShowCursor();

		bool exit = GuiFloatBox(screenWidth / 4, screenHeight * 1.7 + 10, 150, 50, "<- Back");
		if (exit)
			msms = MenuState;

		bool createNew = GuiFloatBox(screenWidth * 1.75, screenHeight * 1.7 + 10, 150, 50, "Add new ->");
		if (createNew)
			msms = BuildParticleState;

		Vector3 angle = { DEG2RAD * pitch, DEG2RAD * yaw, DEG2RAD * roll };

		EndDrawing();
	}

	Vector2 mousePosition;
	Vector2 windowPosition;
	Vector2 panOffset = mousePosition;
	bool dragWindow;
	bool exitWindow;

	int screenWidth;
	int screenHeight;

	void SetupMainScene()
	{
		mousePosition = { 0 };
		windowPosition = { 500, 200 };
		panOffset = mousePosition;
		dragWindow = false;
		exitWindow = false;

		screenWidth = 800;
		screenHeight = 600;
	}

	bool GuiFloatBox(int width, int height, int sizeX, int sizeY, const char* text)
	{
		bool isPressed = false;

		Rectangle bounds = Rectangle((width - sizeX) / 2.0f, (height - sizeY) / 2.0f, (float)sizeX, (float)sizeY);
		GuiState state = STATE_NORMAL;

		if (state != STATE_DISABLED)
		{
			Vector2 mousePoint = GetMousePosition();

			if (CheckCollisionPointRec(mousePoint, bounds))
			{
				state = STATE_FOCUSED;
				if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
				{
					isPressed = true;
				}
			}
		}

		Color baseColor = BLANK;
		Color textColor = Color(225, 225, 225, 255);

		GuiDrawRectangle(bounds, GuiGetStyle(VALUEBOX, BORDER_WIDTH),
			Fade(GetColor(GuiGetStyle(VALUEBOX, BORDER + (state * 3))), guiAlpha), baseColor);

		if (text != NULL)
		{
			GuiDrawText(text, bounds, TEXT_ALIGN_CENTER, textColor);
		}

		return isPressed;
	}

	void MenuScene(State& msms)
	{
		BeginDrawing();
		ClearBackground(BLACK);
		ShowCursor();

		windowPosition.x += (mousePosition.x - panOffset.x);
		windowPosition.y += (mousePosition.y - panOffset.y);

		bool createNew = GuiFloatBox(screenWidth / 4, screenHeight / 4 + 20, 150, 50, "Create new");
		if (createNew)
			msms = State::DisplayParticleState;

		bool openExisting = GuiFloatBox(screenWidth / 4, screenHeight / 4 + 150, 150, 50, "Open existing");
		if (openExisting)
			msms = State::SelectParticleState;

		bool exit = GuiFloatBox(screenWidth / 4, screenHeight / 4 + 280, 150, 50, "Exit");
		if(exit)
			CloseWindow();

		EndDrawing();
	}

	void ParticlePicker(State& msms)
	{
		BeginDrawing();
		ClearBackground(BLACK);
		ShowCursor();

		windowPosition.x += (mousePosition.x - panOffset.x);
		windowPosition.y += (mousePosition.y - panOffset.y);

		// temp
		bool pressedAny = false;
		for (int i = 1; i <= 3; i++)
		{
			for (int j = 1; j <= 3; j++)
			{
				std::string text = "Particle: " + std::to_string(i * j);
				pressedAny = GuiFloatBox(screenWidth / 4 + (i * 220), screenHeight / 4 + (j * 120), 100, 50, text.c_str());
				if(pressedAny)
					msms = State::DisplayParticleState;
			}
		}

		EndDrawing();
	}

	void ParticleBuilder(State& msms)
	{
		BeginDrawing();
		ClearBackground(BLACK);
		ShowCursor();

		windowPosition.x += (mousePosition.x - panOffset.x);
		windowPosition.y += (mousePosition.y - panOffset.y);

		bool createNew = GuiFloatBox(screenWidth / 4, screenHeight / 4 + 20, 150, 50, "Create new emitter");
		bool openExisting = GuiFloatBox(screenWidth / 4, screenHeight / 4 + 150, 150, 50, "Add property");

		bool exit = GuiFloatBox(screenWidth / 4, screenHeight / 4 + 280, 150, 50, "Exit");
		if (exit)
			CloseWindow();

		EndDrawing();
	}
};



