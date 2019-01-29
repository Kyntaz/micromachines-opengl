#include "CollisionObject.h"



CollisionObject::CollisionObject(){


}
void CollisionObject::defineInitialPosition() {
	initialPosition.x = position.x;
	initialPosition.y = position.y;
	initialPosition.z = position.z;
}

void CollisionObject::resetPosition() {
	moveTo(initialPosition.x, initialPosition.y, initialPosition.z);
	movement.direction = 0;
	movement.speed = 0;
	active = true;

}
bool CollisionObject::isColliding(CollisionObject* other) {
	v3 p1 = {
			position.x + collisionBox.center.x,
			position.y + collisionBox.center.y,
			position.z + collisionBox.center.z
	};
	v3 p2 = {
			other->position.x + other->collisionBox.center.x,
			other->position.y + other->collisionBox.center.y,
			other->position.z + other->collisionBox.center.z
	};
		
	float distance = sqrt(
		pow(p1.x - p2.x, 2) +
		pow(p1.y - p2.y, 2) +
		pow(p1.z - p2.z, 2)
	);

	return distance <= collisionBox.radius + other->collisionBox.radius;
}


