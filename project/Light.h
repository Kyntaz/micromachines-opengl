#pragma once

#include "AppObject.h"

class Light : public AppObject {
public:
	Light(vec3 position);

	void initializeObject() override;

};

