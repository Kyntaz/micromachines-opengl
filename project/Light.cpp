#include "Light.h"
#include "basic_geometry.h"


Light::Light(vec3 position){
	moveTo(position.x, position.y, position.z);
}

void Light::initializeObject() {
	Material material = {
		{ 0.3f, 0.31f, 0.3f, 1.0f },
		{ 0.98f, 1.0f, 0.98f, 1.0f },
		{ 0.9f, 0.9f, 0.9f, 1.0f },
		{0.0f, 0.0f, 0.0f, 1.0f},
		500.0,
		0.5f,
		0
	};

	Material pausinhoMaterial = {
	{ 0.05f, 0.05f, 0.05f, 1.0f },
	{ 0.3f, 0.3f, 0.3f, 1.0f },
	{ 0.9f, 0.9f, 0.9f, 1.0f },
	{0.0f, 0.0f, 0.0f, 1.0f},
	1.f,
	1.0f,
	0
	};


	loadIdentity(MODEL);

	auto drawLight = []() {
		createCylinder(5, 3, 10);
	};

	drawObject(material, drawLight);

	auto drawPausinho = []() {
		createCylinder(1, 1, 6);
		::translate(MODEL, 0, -9, 0);
		::scale(MODEL, 1.2, 14, 1.2);

	};

	drawObject(pausinhoMaterial, drawPausinho);
}