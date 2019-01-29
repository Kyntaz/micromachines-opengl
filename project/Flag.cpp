#include "Flag.h"
#include "AVTmathLib.h"

Flag::Flag(float x, float z)
{
	moveTo(x, 5, z);
	billboard = true;
	texture_id1 = 2;
}

void Flag::initializeObject() {
	Material flagMaterial = {
		{  0.8, 0.6, 0.87, 1 },
		{  0.8, 0.6, 0.87, 1 },
		{  0.8, 0.6, 0.87, 1 },
		{0.0f, 0.0f, 0.0f, 1.0f},
		100.0f,
		1.0f,
		1
	};

	loadIdentity(MODEL);
	auto drawFlag = []() {
		::createQuad(5, 10);
	};

	drawObject(flagMaterial, drawFlag);
}

