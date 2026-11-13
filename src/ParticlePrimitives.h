#pragma once

#include "raylib.h"

#include "raymath.h"
#include <vector>
#include <queue>

#include <random>

float GetRandomFloatRange(float low, float high)
{
	static std::mt19937 gen(std::random_device{}());
	std::uniform_real_distribution<float> dist(low, high);
	return dist(gen);
}

struct PropertyBase;

struct BaseParticle
{

	BaseParticle(float _scale, Vector3 _pos, Color _clr)
	{
		this->originalScale = _scale;
		this->scale = _scale;
		this->position = _pos;
		this->color = _clr;
	}

	BaseParticle* clone()
	{
		return new BaseParticle(*this);
	}

	void Draw()
	{
		DrawCube(this->position, this->scale, this->scale, this->scale, this->color);
	}

	void SetLifeTime(float _lifeTime)
	{
		this->lifeTime = _lifeTime;
		this->startingLifeTime = _lifeTime;
	}

	void SetColor(Color _color)
	{
		this->color = _color;
	}

	void SetStartingForce(Vector3 _force)
	{
		this->force = _force;
	}

	void AttatchProperty(PropertyBase* pb)
	{
		properties.push_back(pb);
	}

	void DetatchProperty(PropertyBase* pb)
	{
		auto it = std::find(properties.begin(), properties.end(), pb);
		properties.erase(it);
	}

	void ResetParticle()
	{
		//force = { 0, 0, 0 };
		scale = originalScale;
	}
	
	std::vector<PropertyBase*> properties;

	Vector3 position = {0, 0, 0};
	Vector3 force = {0, 0, 0};
	Vector3 acceleration = {0, 0, 0};

	float startingLifeTime = 0;
	float lifeTime = 0;
	bool shouldDie = true;
	bool activated = false;
	Color color;

	float originalScale = 2;
	float scale = 1;
	int idx = 0;
};

// Due to circular dependencies we have to put properties header down here :(
#include "Properties.h"

struct ParticleEmitter
{
	std::vector<BaseParticle*> particles;

	void AddParticle(Vector3 _pos, Color _clr, bool shouldDie = true)
	{
		BaseParticle* cp = new BaseParticle(0.3f, _pos, _clr);
		particles.push_back(cp);
		cp->idx = particles.size() - 1;
		cp->shouldDie = shouldDie;
		cp->activated = true;
	}

	void RenderAllParticles(FlockbehaviorProperty* flockProp = nullptr)
	{
		for (auto& particle : particles)
		{
			if (particle->activated)
				particle->Draw();

			for (auto& property : particle->properties)
			{
				property->UpdateTarget(particle);
			}
		}

		if (flockProp != nullptr)
		{
			DrawSphere(flockProp->target, 1.0f, PURPLE);
		}
	}

	void SpawnParticle(bool shouldDie = true, Color color = {200, 0, 0, 0})
	{
		int x = GetRandomValue(-10, 10);
		int y = GetRandomValue(-1, 1);
		int z = GetRandomValue(-10, 10);

		AddParticle({ (float)x, (float)y, (float)z }, color, shouldDie);
	}

	void SpawnParticles(int amount = 1, bool shoulDie = true)
	{
		for (int i = 0; i < amount; i++)
		{
			SpawnParticle(shoulDie);
		}
	}

	void ManageParticles()
	{
		int particlesToSpawn = 0;
		std::queue<BaseParticle*> tempHolder;

		for (int i = 0; i < this->particles.size(); ++i)
		{
			this->particles[i]->idx = i;
		}

		std::erase_if(this->particles, [&particlesToSpawn, &tempHolder, this](auto particle)
		{
			if (particle->lifeTime <= 0 && !particle->activated)
				return false;

			particle->lifeTime -= GetFrameTime();

			if (particle->shouldDie)
			{
				if (particle->lifeTime <= 0 && particle->shouldDie)
				{
					++particlesToSpawn;
					tempHolder.push(particle->clone());
					delete particle;
					return true;
				}
			}

			particle->ResetParticle();

			for (auto& property : particle->properties)
			{
				property->Update(particle, GetFrameTime(), this->particles);
			}

			return false;
		});

		for (int i = 0; i < particlesToSpawn; ++i)
		{
			BaseParticle* front = tempHolder.front();
			this->SpawnParticle(front->shouldDie, front->color);

			auto particle = this->particles[this->particles.size() - 1];
			particle->activated = true;
			particle->SetLifeTime(GetRandomFloatRange(1.0f, 2.0f));

			for (auto& property : front->properties)
			{
				particle->properties.push_back(property);
			}

			delete front;
			tempHolder.pop();
		}
	}
};