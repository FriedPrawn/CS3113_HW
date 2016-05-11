#pragma once

#include "Entity.h"

class Player : public Entity
{
public:
	Player();
	void update(float elapsed, float friction_x, float gravity_y);
	bool isAlive;
	int hitpoints;
};