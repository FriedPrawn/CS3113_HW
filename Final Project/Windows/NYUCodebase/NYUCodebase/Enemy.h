#pragma once

#include "Entity.h"

enum EnemyType
{
	ENEMY_WORM
};
enum State
{
	PATROL, IDLE, ANGRY
};
class Enemy : public Entity
{
public:
	Enemy();

	EnemyType type;
	State state;
};