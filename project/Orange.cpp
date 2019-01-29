#include "Orange.h"
#include "AVTmathLib.h"


Orange::Orange(){
	collisionBox.center = { 0, 5.0f, 0 };
	collisionBox.radius = 5.0f;
}

Orange::Orange(struct orangePhysics physics) {
	orangePhysics = physics;
}


void Orange::initializeObject() {
	Material orangeMaterial = {
		{  0.73f, 0.32f, 0.03f, 1.0f },
		{  1.0f, 0.63f, 0.39f, 1.0f },
		{  1.0f, 1.0f, 1.0f, 1.0f },
		{0.0f, 0.0f, 0.0f, 1.0f},
		500.0,
		1.0f,
		0
	};

	loadIdentity(MODEL);
	auto drawOrange = [this]() {
		::createSphere(collisionBox.radius, 10);
		::translate(MODEL, collisionBox.center.x, collisionBox.center.y, collisionBox.center.z);
	};

	drawObject(orangeMaterial, drawOrange);
}

