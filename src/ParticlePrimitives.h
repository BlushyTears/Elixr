#pragma once

#include "raylib.h"

#include "raymath.h"
#include <vector>

struct BaseParticle
{
	virtual ~BaseParticle() = default;
	virtual void Draw() = 0;
	
	Vector3 position;

	//Color color;
	Vector3 force = {0, 0, 0};
	Vector3 acceleration = {0, 0, 0};

	float lifeTime = 1.5;
	Color color;

	int scale = 1;
};

struct CubeParticle : BaseParticle
{
	CubeParticle(int _scale, Vector3 _pos, Color _clr)
	{
		this->scale = _scale;
		this->position = _pos;
		this->color = _clr;
	}

	void Draw() override
	{
		DrawCube(this->position, this->scale, this->scale, this->scale, this->color);
	}
};

struct SphereParticle : BaseParticle
{
	SphereParticle(int _scale, Vector3 _pos, Color _clr)
	{
		this->scale = _scale;
		this->position = _pos;
		this->color = _clr;
	}
	void Draw() override
	{
		DrawSphere(this->position, this->scale, color);
	}
};

struct ParticleManager {
	std::vector<BaseParticle*> particles;

	void AddCubeParticle(Vector3 _pos, Color _clr)
	{
		CubeParticle* cp = new CubeParticle(1, _pos, _clr);
		particles.push_back(cp);
	}

	void AddSphereParticle(Vector3 _pos, Color _clr)
	{
		SphereParticle* sp = new SphereParticle(1, _pos, _clr);
		particles.push_back(sp);
	}

	void RenderAllParticles()
	{
		for (auto& particle : particles)
		{
			particle->Draw();
		}
	}
};