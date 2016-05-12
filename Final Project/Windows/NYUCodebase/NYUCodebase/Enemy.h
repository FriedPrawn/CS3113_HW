#pragma once

#include "Entity.h"

enum EnemyType
{
	ENEMY_WORM
};
enum Statez
{
	E_PATROL, E_NORMAL, E_ANGRY
};
class Enemy : public Entity
{
public:
	Enemy();
	void detectEntity(Entity* target, float tileRange);
	EnemyType type;
	Statez state;
};