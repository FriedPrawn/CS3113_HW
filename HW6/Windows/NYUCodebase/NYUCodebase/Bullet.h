#pragma once

#include "Entity.h"

class Bullet : public Entity
{
public:
	Bullet();
	void colliding(Entity* target);
	void update(float elapsed);
	float timeAlive;
	EntityType source;
};