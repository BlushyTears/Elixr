#pragma once


#include "raylib.h"

#include "raymath.h"
#include <vector>
#include <queue>

#include <random>

#include "ParticlePrimitives.h"

// A module that describes some sort of behavior to a particle ecs style
struct PropertyBase
{
	virtual ~PropertyBase() = default;
	virtual void Update(BaseParticle* pm, float dt, std::vector<BaseParticle*>& particles) = 0;
	virtual void UpdateTarget(BaseParticle* pm) {};
};

struct WaveProperty : PropertyBase
{
	Vector3 baseForce = Vector3(0, 0.01, 0);
	float amplitude = 0.05f;
	float freq = 3.0f;
	float cap = 1.0f;

	void Update(BaseParticle* pm, float dt, std::vector<BaseParticle*>& particles) override
	{
		float globalTime = (float)GetTime();
		float uniqueSeed = (float)((uintptr_t)pm % 1000) * 0.1f;

		float angle = (globalTime * freq) + uniqueSeed;

		Vector3 wave = Vector3(cosf(angle), 0, sinf(angle));
		pm->force += baseForce + wave;
		pm->acceleration += Vector3(Clamp(pm->force.x, 0, cap), Clamp(pm->force.y, 0, cap), Clamp(pm->force.z, 0, cap));
		pm->position += pm->acceleration * GetFrameTime();
	}
};

struct SingleForceProperty : PropertyBase
{
	Vector3 baseForce = Vector3(0, 0.1, 0);
	float cap = 1.0f;

	void Update(BaseParticle* pm, float dt, std::vector<BaseParticle*>& particles) override
	{
		pm->force += baseForce;
		pm->acceleration += Vector3(Clamp(pm->force.x, 0, cap), Clamp(pm->force.y, 0, cap), Clamp(pm->force.z, 0, cap));
		pm->position += pm->acceleration * GetFrameTime();
	}
};

struct ColorChangeProperty : PropertyBase
{
	void Update(BaseParticle* pm, float dt, std::vector<BaseParticle*>& particles) override
	{
		pm->SetColor(Color(GetRandomValue(0, 0), GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255)));
	}
};

struct ColorStaticProperty : PropertyBase
{
	Color color = { 255, 255, 255, 255 };

	void Update(BaseParticle* pm, float dt, std::vector<BaseParticle*>& particles) override
	{
		pm->SetColor(color);
	}
};

struct VortexProperty : PropertyBase
{
	Vector3 center = { 0.0f, 0.0f, 0.0f };
	Vector3 axis = { 0.0f, 1.0f, 0.0f };
	float swirlStrength = 15.0f;
	float pullStrength = 4.0f;
	float liftStrength = 1.5f;
	float maxRadius = 20.0f;

	void Update(BaseParticle* pm, float dt, std::vector<BaseParticle*>& particles) override
	{
		Vector3 toParticle = Vector3Subtract(pm->position, center);

		Vector3 radial = { toParticle.x, 0.0f, toParticle.z };
		float dist = Vector3Length(radial);

		// if distance to center is too small dont do anything
		if (dist < 0.001f || dist > maxRadius)
			return;

		Vector3 radialDir = Vector3Scale(radial, 1.0f / dist);

		Vector3 tangentDir = Vector3CrossProduct(axis, radialDir);

		Vector3 swirlForce = Vector3Scale(tangentDir, swirlStrength);
		Vector3 inwardForce = Vector3Scale(radialDir, -pullStrength);
		Vector3 liftForce = Vector3Scale(axis, liftStrength);

		Vector3 totalForce = Vector3Add(swirlForce, Vector3Add(inwardForce, liftForce));

		pm->force = Vector3Scale(pm->force, 0.98f);
		pm->force = Vector3Add(pm->force, Vector3Scale(totalForce, dt));
		pm->position = Vector3Add(pm->position, Vector3Scale(pm->force, dt));
	}
};

struct FadeoutColorProperty : PropertyBase
{
	void Update(BaseParticle* pm, float dt, std::vector<BaseParticle*>& particles) override
	{
		float alpha = pm->lifeTime / pm->startingLifeTime;

		pm->SetColor(Color(pm->color.r, pm->color.g, pm->color.b, GetRandomValue(1, 255 * alpha)));

		if (pm->lifeTime <= 0.3f)
			pm->scale -= 0.0001f;
	}
};

// Inspiration: https://medium.com/better-programming/mastering-flock-simulation-with-boids-c-opengl-and-imgui-5a3ddd9cb958
struct FlockbehaviorProperty : PropertyBase
{
	Vector3 target = { 0, 0, 0 };
	float updateTimer = 5.0f;
	float currentTimer = updateTimer;

	const float neighboorMaxDist = 3.5f;
	const int maxNeighbors = 15;
	float separationMinDist = 1.2f;
	float minSpeed = 5.0f;
	float maxSpeed = 120.0f;
	float maxSteerForce = 18.0f;
	float cohesionFactor = 0.2f;
	float alignmentFactor = 0.2f;
	float separationFactor = 4.5f;
	float targetFactor = 1.2f;

	void UpdateBoid(BaseParticle* pm, float dt, std::vector<BaseParticle*>& particles)
	{
		if (Vector3LengthSqr(pm->force) < 0.01f)
		{
			Vector3 initDir = {
				GetRandomFloatRange(-1.0f, 1.0f),
				GetRandomFloatRange(-0.5f, 0.5f),
				GetRandomFloatRange(-1.0f, 1.0f)
			};
			if (Vector3LengthSqr(initDir) < 0.0001f) initDir = { 1.0f, 0.0f, 0.0f };
			pm->force = Vector3Scale(Vector3Normalize(initDir), minSpeed);
		}

		Vector3 forward = Vector3Normalize(pm->force);

		int neighborCount = 0;
		Vector3 sumNeighborSpeeds = { 0, 0, 0 };
		Vector3 sumNeighborPositions = { 0, 0, 0 };
		Vector3 separationForce = { 0, 0, 0 };

		const float maxDistSqr = neighboorMaxDist * neighboorMaxDist;

		for (BaseParticle* other : particles)
		{
			if (other == pm || !other->activated)
				continue;

			Vector3 toOther = Vector3Subtract(other->position, pm->position);
			float distSqr = Vector3LengthSqr(toOther);

			if (distSqr < 0.000001f || distSqr > maxDistSqr)
				continue;

			float dist = sqrtf(distSqr);
			Vector3 dirToOther = Vector3Scale(toOther, 1.0f / dist);

			if (Vector3DotProduct(forward, dirToOther) < -0.5f)
				continue;

			neighborCount++;
			sumNeighborSpeeds = Vector3Add(sumNeighborSpeeds, other->force);
			sumNeighborPositions = Vector3Add(sumNeighborPositions, other->position);

			if (dist < separationMinDist)
			{
				float strength = (separationMinDist - dist) / separationMinDist;
				separationForce = Vector3Subtract(separationForce, Vector3Scale(dirToOther, strength));
			}

			if (neighborCount >= maxNeighbors)
				break;
		}

		Vector3 steerForce = { 0, 0, 0 };

		if (neighborCount > 0)
		{
			float invCount = 1.0f / (float)neighborCount;
			Vector3 avgPos = Vector3Scale(sumNeighborPositions, invCount);
			Vector3 avgSpeed = Vector3Scale(sumNeighborSpeeds, invCount);

			Vector3 cohesion = Vector3Scale(Vector3Subtract(avgPos, pm->position), cohesionFactor);
			Vector3 alignment = Vector3Scale(Vector3Subtract(avgSpeed, pm->force), alignmentFactor);

			steerForce = Vector3Add(steerForce, cohesion);
			steerForce = Vector3Add(steerForce, alignment);
		}

		steerForce = Vector3Add(steerForce, Vector3Scale(separationForce, separationFactor));

		Vector3 targetOffset = Vector3Subtract(target, pm->position);
		float distToTarget = Vector3Length(targetOffset);
		if (distToTarget > 0.001f)
		{
			Vector3 desiredDir = Vector3Scale(targetOffset, 1.0f / distToTarget);
			float weight = (distToTarget < 6.0f) ? (distToTarget / 6.0f) * 0.4f : 1.0f;
			Vector3 desiredVel = Vector3Scale(desiredDir, maxSpeed);
			Vector3 targetSteer = Vector3Scale(Vector3Subtract(desiredVel, pm->force), targetFactor * weight);
			steerForce = Vector3Add(steerForce, targetSteer);
		}

		float steerMag = Vector3Length(steerForce);
		if (steerMag > maxSteerForce)
		{
			steerForce = Vector3Scale(steerForce, maxSteerForce / steerMag);
		}

		pm->force = Vector3Add(pm->force, Vector3Scale(steerForce, dt));

		float speed = Vector3Length(pm->force);
		if (speed > maxSpeed)
		{
			pm->force = Vector3Scale(pm->force, maxSpeed / speed);
		}
		else if (speed < minSpeed && speed > 0.0001f)
		{
			pm->force = Vector3Scale(pm->force, minSpeed / speed);
		}

		pm->position = Vector3Add(pm->position, Vector3Scale(pm->force, dt));
	}

	void MoveTarget()
	{
		target.x = GetRandomFloatRange(-20, 20);
		target.y = GetRandomFloatRange(-8, 12);
		target.z = GetRandomFloatRange(-20, 20);
	}

	void Update(BaseParticle* pm, float dt, std::vector<BaseParticle*>& particles) override
	{
		UpdateBoid(pm, dt, particles);

		if (pm->idx == 0)
		{
			currentTimer -= dt;
			if (currentTimer <= 0)
			{
				MoveTarget();
				currentTimer = updateTimer;
			}
		}
	}
};