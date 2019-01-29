#pragma once

#include <vector>
#include "basic_geometry.h"
#include "AVTmathLib.h"

// From lightDemo.cpp
extern struct MyMesh mesh[];
extern int objId;
extern int maxObjId;

// A tripple of values
typedef struct v3 {
	float x;
	float y;
	float z;
} vec3;

// A single object
class AppObject {
public:
	// The position of the mesh in the mesh array.
	std::vector<int> meshes;

	// Transformations
	struct v3 position;
	struct v3 scale;
	struct v3 rotation;

	// This controls whether the object is a billboard.
	bool billboard = false;

	// The texture ids the object uses (second is for multitexturing).
	int texture_id1 = 0;
	int texture_id2 = 1;

	// This controls whether the object should be rendered.
	bool active = true;

	AppObject();

	void moveTo(float x, float y, float z);
	void resize(float x, float y, float z);
	void step(float x, float y, float z);
	void rotate(float x, float y, float z);
	void addMesh(int meshId);

	virtual void initializeObject() {};

protected:
	template<typename Functor>
	void drawObject(Material material, Functor &f) {
		objId = maxObjId++;
		memcpy(mesh[objId].mat.ambient, material.ambient, 4 * sizeof(float));
		memcpy(mesh[objId].mat.diffuse, material.diffuse, 4 * sizeof(float));
		memcpy(mesh[objId].mat.specular, material.specular, 4 * sizeof(float));
		memcpy(mesh[objId].mat.emissive, material.emissive, 4 * sizeof(float));
		mesh[objId].mat.shininess = material.shininess;
		mesh[objId].mat.transparency = material.transparency;
		mesh[objId].mat.texCount = material.texCount;
				
		pushMatrix(MODEL);
		f();
		memcpy(mesh[objId].transform, get(MODEL), 16 * sizeof(float));
		popMatrix(MODEL);

		addMesh(objId);
	}
};
