#pragma once

#include "raylib.h"

#include "raymath.h"

struct CameraController
{
	CameraController()
	{
		cam = { 0 };
		cam.position = Vector3(0.0f, 5.0f, -36.0f);
		cam.target = Vector3(0.0f, 0.0f, 0.0f);
		cam.up = Vector3(0.0f, 1.0f, 0.0f);
		cam.fovy = 60.0f;
		cam.projection = CAMERA_PERSPECTIVE;
	}

	void Freecam()
	{
		if (IsKeyDown(KEY_W))
			cam.position.z += 1.0f;
		if (IsKeyDown(KEY_S))
			cam.position.z -= 1.0f;
		if (IsKeyDown(KEY_A))
			cam.position.x += 1.0f;
		if (IsKeyDown(KEY_D))
			cam.position.x -= 1.0f;
		if (IsKeyDown(KEY_Q))
			cam.position.y += 1.0f;
		if (IsKeyDown(KEY_E))
			cam.position.y -= 1.0f;
	}

	Camera cam;
};