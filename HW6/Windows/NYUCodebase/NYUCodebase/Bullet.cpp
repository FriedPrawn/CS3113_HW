#include "Bullet.h"


Bullet::Bullet() : 	Entity(-2000.0f, -2000.0f, 0.0f, 0, 0.010f, 0.1f, 0.0f, 0.0f, 0.0f)
{
	//Entity();
	timeAlive = 0.0f;
	/*x = -2000.0f;
	y = -2000.0f;
	rotation = 0.0f;
	textureID = 0;
	width = 0.1f;
	height = 0.1f; 
	speed = 0.0f; 
	direction_x = 0.0f;
	direction_y = 0.0f;*/

};
void Bullet::colliding(Entity* target)
{
	float targetRight = target->x + target->width * 0.5f;
	float targetLeft = target->x - target->width * 0.5f;
	float targetTop = target->y + target->height * 0.5f;;
	float targetBot = target->y - target->height * 0.5f;
	
	float right = x + width * 0.5f;
	float left = x - width * 0.5f;
	float top = y + width * 0.5f;
	float bot = y - width * 0.5f;
	/*if (isAlive && x <= (target->x + target->width * 0.5f) && x > (target->x - target->width * 0.5f) && y >= (target->y - target->height * 0.5f) && y <= (target->y + target->height * 0.5f) && target->isAlive)*/
	if ( !(bot > targetTop || top < targetBot || left > targetRight || right < targetLeft ) && isAlive && target->isAlive && source != target->role)
	{
		target->isAlive = false;
		isAlive = false;
		//return true;
	}
//	return false;
}

void Bullet::update(float elapsed)
{
	x += direction_x * elapsed * speed;
	y += direction_y * elapsed * speed;
	timeAlive += elapsed;
	if (y > 2.0f || y< -2.0f)
		isAlive = false;
	if (!isAlive)
	{
		x = 2000.0f;
		y = 2000.0f;
		direction_y = 0.0f;
		direction_x = 0.0f;
		speed = 0.0f;
		//timeAlive = 0.0f;
	}
}
