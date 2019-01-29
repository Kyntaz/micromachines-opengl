#pragma once
#include "CollisionObject.h"

class Cheerio : public CollisionObject {
public:
	Cheerio(float scale);

	void initializeObject() override;

private:
	float scale;
};

