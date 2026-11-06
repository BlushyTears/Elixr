#pragma once

#include "raylib.h"

void moveModel(float& pitch, float& yaw, float& roll)
{
	if (IsKeyDown(KEY_DOWN))
		pitch += 0.6f;
	else if 
		(IsKeyDown(KEY_UP)) pitch -= 0.6f;
	else
	{
		if 
			(pitch > 0.3f) pitch -= 0.3f;
		else
			if (pitch < -0.3f) pitch += 0.3f;
	}
	if (IsKeyDown(KEY_LEFT))
		roll -= 1.0f;
	else if 
		(IsKeyDown(KEY_RIGHT)) roll += 1.0f;
	else
	{
		if 
			(roll > 0.0f) roll -= 0.5f;
		else if
			(roll < 0.0f) roll += 0.5f;
	}
}