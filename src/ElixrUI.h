#pragma once

#include <iostream>
#include <string>
#include <algorithm>

#include "raylib.h"
#include "raymath.h"
#include "resource_dir.h"

#include "ElixrUtil.h"

#include <ranges>

int screenWidth = 1600;
int screenHeight = 1200;

inline bool GuiFloatBox(int width, int height, int sizeX, int sizeY, const char* text);
inline bool GuiFloatBoxBP(int width, int height, int sizeX, int sizeY, const char* text);

struct EmitterBlueprint
{
	Vector2 position;
	// is mouse/joycon holding this, and if so we wanna be able to drag it
	bool isHeld = false;
	ParticleEmitter* emitterReference = nullptr;
	std::string name = "Emitter";
};

struct PropertyBlueprint
{
	Vector2 position;
	// is mouse/joycon holding this, and if so we wanna be able to drag it
	bool isHeld = false;
	ParticleEmitter* emitterReference = nullptr;
	std::string name = "Property";
};

struct BlueprintBuilder
{
	std::vector<EmitterBlueprint> emitters;
	std::vector<PropertyBlueprint> properties;

	void AddNewEmitter()
	{
		EmitterBlueprint eb;
		eb.position = Vector2(screenWidth, screenHeight);
		eb.name = "Something";
		emitters.push_back(eb);
	}

	void AddNewProperty()
	{
		PropertyBlueprint pb;
		pb.position = Vector2(screenWidth, screenHeight);
		properties.push_back(pb);
	}

	void DisplayAllBlueprints()
	{
		auto process = [&](auto& item)
			{
				using ItemType = std::decay_t<decltype(item)>;


				if constexpr (std::is_same_v<ItemType, EmitterBlueprint>)
				{
					item.isHeld = GuiFloatBoxBP(static_cast<int>(item.position.x),
						static_cast<int>(item.position.y), 150, 50, item.name.c_str());
				}
				if constexpr (std::is_same_v<ItemType, PropertyBlueprint>)
				{
					item.isHeld = GuiFloatBoxBP(static_cast<int>(item.position.x),
						static_cast<int>(item.position.y), 150, 50, item.name.c_str());
				}
				if (item.isHeld)
				{
					item.position = Vector2(GetMousePosition().x * 2, GetMousePosition().y * 2);
				}
			};

		std::ranges::for_each(emitters, process);
		std::ranges::for_each(properties, process);
	}
};