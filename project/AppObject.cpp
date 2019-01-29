#include "AppObject.h"
#include "AVTmathLib.h"


AppObject::AppObject() {
	position = { 0, 0, 0 };
	scale = { 1, 1, 1 };
	rotation = { 0, 0, 0 };
}

void AppObject::moveTo(float x, float y, float z) {
	position = { x, y, z };
}

void AppObject::resize(float x, float y, float z) {
	scale.x *= x;
	scale.y *= y;
	scale.z *= z;
}

void AppObject::step(float x, float y, float z) {
	position.x += x;
	position.y += y;
	position.z += z;
}

void AppObject::rotate(float x, float y, float z) {
	rotation = { x, y, z };
}

void AppObject::addMesh(int meshId) {
	meshes.push_back(meshId);
}


