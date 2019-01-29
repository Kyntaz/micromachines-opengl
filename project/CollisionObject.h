#pragma once
#include "AppObject.h"

class CollisionObject :
	public AppObject
{
public:
	struct {
		float direction;
		float speed;
		float drag;
	} movement;

	struct v3 initialPosition;

	CollisionObject();

	void defineInitialPosition();

	bool isColliding(CollisionObject* other);

	void resetPosition();
protected:
	struct {
		float radius;
		vec3 center;
	} collisionBox;
};

