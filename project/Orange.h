#pragma once
#include "CollisionObject.h"

struct orangePhysics {
	float speedX;
	float speedZ;

	float rotationX;
	float rotationZ;

	float timeToRespawn;
};

class Orange : public CollisionObject {
public:
	orangePhysics orangePhysics;

	Orange();

	Orange(struct orangePhysics physics);
	
	void initializeObject() override;
};

