#pragma once

#include "raylib.h"

#include "raymath.h"
#include <vector>
#include <queue>

struct PropertyBase;

struct BaseParticle
{
	virtual ~BaseParticle() = default;
	virtual void Draw() = 0;
	virtual BaseParticle* clone() = 0;

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
		force = { 0, 0, 0 };
		scale = originalScale;
	}
	
	std::vector<PropertyBase*> properties;

	Vector3 position = {0, 0, 0};
	Vector3 force = {0, 0, 0};
	Vector3 acceleration = {0, 0, 0};

	float startingLifeTime = 0;
	float lifeTime = 0;
	bool activated = false;
	Color color;

	float originalScale = 2;
	float scale = 1;
};

struct CubeParticle : BaseParticle
{
	CubeParticle(float _scale, Vector3 _pos, Color _clr)
	{
		this->originalScale = _scale;
		this->scale = _scale;
		this->position = _pos;
		this->color = _clr;
	}

	BaseParticle* clone() override
	{
		return new CubeParticle(*this);
	}

	void Draw() override
	{
		DrawCube(this->position, this->scale, this->scale, this->scale, this->color);
	}
};

struct SphereParticle : BaseParticle
{
	SphereParticle(float _scale, Vector3 _pos, Color _clr)
	{
		this->originalScale = _scale;
		this->scale = _scale;
		this->position = _pos;
		this->color = _clr;
	}

	BaseParticle* clone() override
	{
		return new SphereParticle(*this);
	}

	void Draw() override
	{
		DrawSphere(this->position, this->scale, color);
	}
};

// A module that describes some sort of behavior to a particle ecs style
struct PropertyBase
{
	virtual ~PropertyBase() = default;
	virtual void Update(BaseParticle* pm) = 0;
};

struct WaveProperty : PropertyBase
{
	Vector3 baseForce = Vector3(0, 0.01, 0);
	float amplitude = 0.05f;
	float freq = 3.0f;

	void Update(BaseParticle* pm) override
	{
		float globalTime = (float)GetTime();
		float uniqueSeed = (float)((uintptr_t)pm % 1000) * 0.1f;

		float angle = (globalTime * freq) + uniqueSeed;

		Vector3 wave = Vector3(cosf(angle), 0, sinf(angle));
		pm->force += baseForce + wave;
		pm->acceleration += pm->force;
		pm->position += pm->acceleration * GetFrameTime();
	}
};

struct UpforceProperty : PropertyBase
{
	Vector3 baseForce = Vector3(0, 0.1, 0);

	void Update(BaseParticle* pm) override
	{
		pm->force += baseForce;
		pm->acceleration += pm->force;
		pm->position += pm->acceleration * GetFrameTime();
	}
};

struct ColorChangeProperty : PropertyBase
{
	void Update(BaseParticle* pm) override
	{
		pm->SetColor(Color( GetRandomValue(0, 0), GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255) ));
	}
};

struct FadeoutColorProperty : PropertyBase
{
	void Update(BaseParticle* pm) override
	{
		float alpha = pm->lifeTime / pm->startingLifeTime;

		pm->SetColor(Color(pm->color.r, pm->color.g, pm->color.b, GetRandomValue(1, 255 * alpha)));
		
		if (pm->lifeTime <= 0.3f)
			pm->scale -= 0.0001f;
	}
};

struct ParticleSpawner
{
	std::vector<BaseParticle*> particles;

	void AddCubeParticle(Vector3 _pos, Color _clr)
	{
		CubeParticle* cp = new CubeParticle(0.3f, _pos, _clr);
		particles.push_back(cp);
	}

	void AddSphereParticle(Vector3 _pos, Color _clr)
	{
		SphereParticle* sp = new SphereParticle(0.3f, _pos, _clr);
		particles.push_back(sp);
	}

	void RenderAllParticles()
	{
		for (auto& particle : particles)
		{
			if(particle->activated)
				particle->Draw();
		}
	}

	void SpawnParticle(Color color = {200, 0, 0, 0})
	{
		int x = GetRandomValue(-10, 10);
		int y = GetRandomValue(-1, 1);
		int z = GetRandomValue(-10, 10);

		AddCubeParticle({ (float)x, (float)y, (float)z }, color);
	}

	void SpawnParticles(int amount = 1)
	{
		for (int i = 0; i < amount; i++)
		{
			SpawnParticle();
		}
	}

	void ManageParticles()
	{
		int particlesToSpawn = 0;
		std::queue<BaseParticle*> tempHolder;
		
		std::erase_if(this->particles, [&particlesToSpawn, &tempHolder, this](auto particle)
		{
			if (!particle->lifeTime)
				return false;

			particle->lifeTime -= GetFrameTime();

			if (particle->lifeTime <= 0)
			{
				++particlesToSpawn;

				BaseParticle* ref = particle;
				tempHolder.push(ref->clone());

				delete particle;
				return true;
			}

			particle->ResetParticle();

			for (auto& property : particle->properties)
			{
				property->Update(particle);
			}

			return false;
		});

		for (int i = 0; i < particlesToSpawn; ++i)
		{
			BaseParticle* front = tempHolder.front();
			this->SpawnParticle(front->color);
			
			auto particle = this->particles[this->particles.size() - 1];
			particle->activated = true;

			particle->SetLifeTime(front->startingLifeTime);
			tempHolder.pop();

			// re-attatch the same properties to newly spawned particle
			for (auto& property : front->properties)
			{
				particle->properties.push_back(property);
			}

			delete front;
		}
	}
};