#pragma once

#include <iostream>

#include "raylib.h"
#include "raymath.h"
#include "resource_dir.h"

#include "ModelLoading.h"
#include "CameraController.h"
#include "ParticlePrimitives.h"
#include "ElixrUI.h"
#include "SceneStates.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

unsigned int animIndex = 0;
unsigned int animCurrentFrame = 0;

inline bool GuiFloatBoxBP(int width, int height, int sizeX, int sizeY, const char* text)
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
			if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
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

inline bool GuiFloatBox(int width, int height, int sizeX, int sizeY, const char* text)
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

inline void PlayIdleAnimation(Model& model, ModelAnimation* anims, int animCount)
{
	if (anims == nullptr || animCount <= 0 || animIndex >= (unsigned int)animCount)
		return;

	animCurrentFrame++;
	if (animCurrentFrame >= anims[animIndex].keyframeCount)
	{
		animCurrentFrame = 0;
	}
	UpdateModelAnimation(model, anims[animIndex], animCurrentFrame);
}

inline static void DrawModelSkeleton(ModelSkeleton skeleton, ModelAnimPose pose, float scale, Color color)
{
	// Loop to (boneCount - 1) because the last one is a special "no bone" bone,
	// needed to workaround buggy models without a -1, a cube is always drawn at the origin
	for (unsigned int i = 0; i < skeleton.boneCount - 1; i++)
	{
		DrawCube(pose[i].translation, scale * 0.05f, scale * 0.05f, scale * 0.05f, color);

		if (skeleton.bones[i].parent >= 0)
		{
			DrawLine3D(pose[i].translation, pose[skeleton.bones[i].parent].translation, color);
		}
	}
}

inline Vector3 GetScaledVertex(const float* verts, int index, float scale)
{
	Vector3 v = {
		verts[index * 3 + 0],
		verts[index * 3 + 1],
		verts[index * 3 + 2]
	};
	return Vector3Scale(v, scale);
}

inline void AddEdgePoints(Vector3 start, Vector3 end, int steps, std::vector<float>& outPositions)
{
	for (int i = 0; i < steps; i++)
	{
		float t = (float)i / (float)steps;
		Vector3 point = Vector3Lerp(start, end, t);

		outPositions.push_back(point.x);
		outPositions.push_back(point.y);
		outPositions.push_back(point.z);
	}
}

inline void UpdateEdgePoints(Vector3 start, Vector3 end, int steps, std::vector<float>& outPositions, int& idx)
{
	for (int i = 0; i < steps; i++)
	{
		float t = (float)i / (float)steps;
		Vector3 point = Vector3Lerp(start, end, t);

		outPositions[idx++] = point.x;
		outPositions[idx++] = point.y;
		outPositions[idx++] = point.z;
	}
}

inline void SetParticlePositions(int modelCount, float modelScale, const Model& model,
	std::vector<float>& vertexPositions)
{
	bool firstTime = vertexPositions.empty();
	const int steps = 4;
	int idx = 0;

	for (int m = 0; m < modelCount; m++)
	{
		const Mesh& mesh = model.meshes[m];
		const float* verts = mesh.animVertices ? mesh.animVertices : mesh.vertices;
		int triCount = mesh.indices ? mesh.triangleCount : (mesh.vertexCount / 3);

		for (int t = 0; t < triCount; t++)
		{
			int i0 = mesh.indices ? mesh.indices[t * 3 + 0] : (t * 3 + 0);
			int i1 = mesh.indices ? mesh.indices[t * 3 + 1] : (t * 3 + 1);
			int i2 = mesh.indices ? mesh.indices[t * 3 + 2] : (t * 3 + 2);

			Vector3 a = GetScaledVertex(verts, i0, modelScale);
			Vector3 b = GetScaledVertex(verts, i1, modelScale);
			Vector3 c = GetScaledVertex(verts, i2, modelScale);

			if (firstTime)
			{
				AddEdgePoints(a, b, steps, vertexPositions);
				AddEdgePoints(b, c, steps, vertexPositions);
				AddEdgePoints(c, a, steps, vertexPositions);
			}
			else
			{
				UpdateEdgePoints(a, b, steps, vertexPositions, idx);
				UpdateEdgePoints(b, c, steps, vertexPositions, idx);
				UpdateEdgePoints(c, a, steps, vertexPositions, idx);
			}
		}
	}
}

