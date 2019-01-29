#pragma once
#include "AppObject.h"

class Table : public AppObject {
public:
	Table(float width, float height, float thickness);

	void initializeObject() override;

private:
	float width, height, thickness;
};

