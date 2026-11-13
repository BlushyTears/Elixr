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
	Vector3 baseForce = { 0.0f, 0.01f, 0.0f };
	float amplitude = 1.5f;
	float freq = 3.0f;
	float cap = 2.0f;

	void Update(BaseParticle* pm, float dt, std::vector<BaseParticle*>& particles) override
	{
		float globalTime = (float)GetTime();
		float uniqueSeed = (float)((uintptr_t)pm % 1000) * 0.1f;

		float angle = (globalTime * freq) + uniqueSeed;

		Vector3 wave = { cosf(angle) * amplitude, 0.0f, sinf(angle) * amplitude };

		pm->force = baseForce + wave;

		pm->acceleration = {
			Clamp(pm->force.x, -cap, cap),
			Clamp(pm->force.y, -cap, cap),
			Clamp(pm->force.z, -cap, cap)
		};

		pm->position = pm->position + (pm->acceleration * dt);
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

struct RandomColorChangeProperty : PropertyBase
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

struct VelocityColorProperty : PropertyBase
{
	Color slowColor = { 50, 255, 255, 255 };
	Color fastColor = { 50, 255, 255, 255 };
	float maxSpeed = 15.0f;

	void Update(BaseParticle* pm, float dt, std::vector<BaseParticle*>& particles) override
	{
		float t = Vector3Length(pm->force) / maxSpeed;
		pm->SetColor(ColorLerp(slowColor, fastColor, t));
	}
};

struct VortexProperty : PropertyBase
{
	Vector3 center = { 0.0f, 0.0f, 0.0f };
	Vector3 axis = { 0.0f, 1.0f, 0.0f };
	float swirlStrength = 15.0f;
	float pullStrength = 4.0f;
	float maxRadius = 20.0f;

	void Update(BaseParticle* pm, float dt, std::vector<BaseParticle*>& particles) override
	{
		Vector3 toParticle = pm->position - center;

		Vector3 radial = { toParticle.x, 0.0f, toParticle.z };
		float dist = Vector3Length(radial);

		// if distance to center is too small dont do anything
		if (dist < 0.001f || dist > maxRadius)
			return;

		// normalize
		Vector3 radialDir = radial * (1.0f / dist);

		Vector3 tangentDir = Vector3CrossProduct(axis, radialDir);

		Vector3 swirlForce = tangentDir * swirlStrength;
		Vector3 inwardForce = radialDir * -pullStrength;

		Vector3 totalForce = swirlForce + inwardForce + axis;

		pm->force = pm->force, 0.98f;
		pm->force = pm->force + totalForce * dt;
		pm->position = pm->position + pm->force * dt;
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

struct DragProperty : PropertyBase
{
	float dragCoefficient = 0.96f;

	void Update(BaseParticle* pm, float dt, std::vector<BaseParticle*>& particles) override
	{
		float damping = powf(dragCoefficient, dt * 60.0f);
		pm->force = pm->force * damping;
		pm->acceleration = pm->acceleration * damping;
	}
};

struct PointAttractorProperty : PropertyBase
{
	Vector3 targetPoint = { 0.0f, 1.0f, 0.0f };
	float strength = 25.0f;
	float maxRadius = 30.0f;
	float minRadius = 0.5f;

	void Update(BaseParticle* pm, float dt, std::vector<BaseParticle*>& particles) override
	{
		Vector3 dir = targetPoint - pm->position;
		float dist = Vector3Length(dir);

		// if we're inside our target bubble
		if (dist > maxRadius || dist < minRadius)
			return;

		Vector3 normDir = dir * (1 / maxRadius);

		float falloff = 1.0f - (dist / maxRadius);

		Vector3 pull = normDir * strength * falloff * dt;

		pm->force = pm->force + pull;
		pm->position = pm->position + pm->force * dt;
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
	float maxSpeed = 25.0f;
	float maxSteerForce = 18.0f;
	float cohesionFactor = 0.2f;
	float alignmentFactor = 0.2f;
	float separationFactor = 4.5f;
	float targetFactor = 1.2f;

	void UpdateBoid(BaseParticle* pm, float dt, std::vector<BaseParticle*>& particles)
	{
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

			Vector3 toOther = other->position - pm->position;
			float distSqr = Vector3LengthSqr(toOther);

			if (distSqr < 0.000001f || distSqr > maxDistSqr)
				continue;

			float dist = sqrtf(distSqr);
			Vector3 dirToOther = toOther / dist;

			if (Vector3DotProduct(forward, dirToOther) < -0.5f)
				continue;

			neighborCount++;
			sumNeighborSpeeds += other->force;
			sumNeighborPositions += other->position;

			if (dist < separationMinDist)
			{
				float strength = (separationMinDist - dist) / separationMinDist;
				separationForce -= dirToOther * strength;
			}

			if (neighborCount >= maxNeighbors)
				break;
		}

		Vector3 steerForce = { 0, 0, 0 };

		// if current particle has a non zero number of neighboors
		if (neighborCount > 0)
		{
			float invCount = 1.0f / (float)neighborCount;
			Vector3 avgPos = sumNeighborPositions * invCount;
			Vector3 avgSpeed = sumNeighborSpeeds * invCount;

			Vector3 cohesion = (avgPos - pm->position) * cohesionFactor;
			Vector3 alignment = (avgSpeed - pm->force) * alignmentFactor;

			steerForce += cohesion + alignment;
		}

		steerForce += separationForce * separationFactor;

		Vector3 targetOffset = target - pm->position;
		float distToTarget = Vector3Length(targetOffset);

		// if we haven't reached our target yet:
		if (distToTarget > 0.001f)
		{
			Vector3 desiredDir = targetOffset / distToTarget;

			float weight;

			if (distToTarget < 6.0f)
				weight = (distToTarget / 6.0f) * 0.4f;
			else
				weight = 1.0f;

			Vector3 desiredVel = desiredDir * maxSpeed;
			Vector3 targetSteer = (desiredVel - pm->force) * (targetFactor * weight);

			steerForce += targetSteer;
		}

		float steerMag = Vector3Length(steerForce);
		if (steerMag > maxSteerForce)
		{
			steerForce *= maxSteerForce / steerMag;
		}

		pm->force += steerForce * dt;

		float speed = Vector3Length(pm->force);
		if (speed > maxSpeed)
		{
			pm->force *= maxSpeed / speed;
		}
		else if (speed < minSpeed && speed > 0.0001f)
		{
			pm->force *= minSpeed / speed;
		}

		pm->position += pm->force * dt;
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