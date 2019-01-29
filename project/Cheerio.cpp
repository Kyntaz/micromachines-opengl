#include "Cheerio.h"
#include "AVTmathLib.h"

Cheerio::Cheerio(float scale){
	this->scale = scale;
	collisionBox.center = { 0, 0.222f * scale, 0 };
	collisionBox.radius = 0.5f * scale;

	movement.speed = 0;
	movement.direction = 0;
	movement.drag = 100;
}


void Cheerio::initializeObject() {
	Material cheerioMaterial = {
		{  0.6f, 0.48f, 0.0f, 1.0f },
		{  1.0f, 0.88f, 0.0f, 1.0f },
		{  1.0f, 1.0f, 1.0f, 1.0f },
		{0.0f, 0.0f, 0.0f, 1.0f},
		500.0,
		1.0f,
		0
	};

	loadIdentity(MODEL);

	auto drawCheerio = [this]() {
		::createTorus(0.222f * scale, 0.5f * scale, 10, 10);
		::translate(MODEL, collisionBox.center.x, collisionBox.center.y, collisionBox.center.z);
	};

	drawObject(cheerioMaterial, drawCheerio);
}

