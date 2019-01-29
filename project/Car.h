#pragma once
#include "AppObject.h"
#include "CollisionObject.h"
#include "vsShaderLib.h"

// assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/color4.h>

class Car : public CollisionObject {
public:
	struct {
		float accel;
		float speed;
		float angle;
		float angularSpeed;
	} carPhysics;

	int lives;
	int laps;
	int fireworks;
	bool doneHalfLap;

	Car();
	
	void initializeObject() override;
	void resetPosition();
	void checkLap(vec3 startPoint, vec3 halfPoint, float radius);

	void resetCar();


private:
	bool isInside(vec3 p, float radius);
	void genVAOsAndUniformBuffer(const aiScene *sc);
};

