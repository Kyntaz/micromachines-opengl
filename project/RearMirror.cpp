#include "RearMirror.h"
#include "AVTmathLib.h"

RearMirror::RearMirror()
{
}

void RearMirror::initializeObject() {
	Material rearMirrorMaterial = {
		{  0.8, 0.6, 0.87, 1 },
		{  0.8, 0.6, 0.87, 1 },
		{  0.8, 0.6, 0.87, 1 },
		{0.0f, 0.0f, 0.0f, 1.0f},
		1000000000000.0f,
		1.0f,
		0
	};

	loadIdentity(MODEL);
	auto drawRearMirror = []() {
		::createQuad(1, 0.5);
	};

	drawObject(rearMirrorMaterial, drawRearMirror);
}

