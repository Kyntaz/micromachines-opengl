#pragma once
#include "AppObject.h"

class Flag : public AppObject {
public:
	Flag(float x, float z);

	void initializeObject() override;
};

