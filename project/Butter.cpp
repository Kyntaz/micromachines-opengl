#include "Butter.h"
#include "AVTmathLib.h"

Butter::Butter()
{
	collisionBox.center = { 0, 0, 0 };
	collisionBox.radius = 9;
	position.x = 0;
	position.z = 0;
	movement.drag = 800;
	initialPosition = position;
}


void Butter::initializeObject() {
	Material butterMaterial = {
		{  0.6f, 0.6f, 0.0f, 1.0f },
		{  1.0f, 1.0f, 0.0f, 1.0f },
		{  1.0f, 1.0f, 1.0f, 1.0f },
		{0.0f, 0.0f, 0.0f, 1.0f},
		500.0,
		1.0f,
		0
	};

	loadIdentity(MODEL);
	auto drawButter = []() {
		::createCube();
		::translate(MODEL, -6, 0.5f, -12);
		::scale(MODEL, 12, 9, 24);

	};

	drawObject(butterMaterial, drawButter);
	defineInitialPosition();
}

