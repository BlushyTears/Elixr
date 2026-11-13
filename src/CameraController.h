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

	Camera cam;
};