#include "Enemy.h"

Enemy::Enemy() : type(ENEMY_WORM), state(E_NORMAL){}

void Enemy::detectEntity(Entity* target, float tileRange)
{
	float bot = y - getHeight() * 0.5f;
	float top = y + getHeight() * 0.5f;
	float left = x - getWidth() * 0.5f;
	float right = x + getWidth() * 0.5f;
	float midHeight = y;

	float targetBot = target->y - target->getHeight() * 0.5f;
	float targetTop = target->y + target->getHeight() * 0.5f;
	float targetLeft = target->x - target->getWidth() * 0.5f;
	float targetRight = target->x + target->getWidth() * 0.5f;

	float detectionRange = tileRange;
	float leftDetect = left - detectionRange;
	float rightDetect = right + detectionRange;

	if (leftDetect < targetRight && leftDetect > targetLeft){
		state = E_ANGRY;
		acceleration_x = -3.0f;
	}


	else if (rightDetect > targetLeft && rightDetect < targetRight && midHeight < targetTop && midHeight > targetBot){
		state = E_ANGRY;
		acceleration_x = 3.0f;
	}

}

