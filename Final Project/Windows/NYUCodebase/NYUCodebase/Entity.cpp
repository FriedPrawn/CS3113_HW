#include "Entity.h"

Entity::Entity(SheetSprite sprite, bool isStatic, float x, float y, float velocity_x, float velocity_y, float acceleration_x, float acceleration_y) :
	sprite(sprite),
	isStatic(isStatic),
	velocity_x(velocity_x),
	velocity_y(velocity_y),
	acceleration_x(acceleration_x),
	acceleration_y(acceleration_y),
	x(x),
	y(y),
	isAlive(true),
	actionState(ACTION_IDLE)
	{}

float Entity::getWidth()
{
	return (sprite.width/sprite.height) * sprite.size;
}
float Entity::getHeight()
{
	return sprite.size;
}
float Entity::lerp(float v0, float v1, float t)
{
	return (1.0 - t)*v0 + t * v1;
}


bool Entity::collidesWith(Entity* entity)
{
	float targetRight = entity->x + entity->getWidth() * 0.5f;
	float targetLeft = entity->x - entity->getWidth() * 0.5f;
	float targetTop = entity->y + entity->getHeight() * 0.5f;
	float targetBot = entity->y - entity->getHeight() * 0.5f;

	float right = x + getWidth() * 0.5f;
	float left = x - getWidth() * 0.5f;
	float top = y + getHeight() * 0.5f;
	float bot = y - getHeight() * 0.5f;
	

	bool collides = !(bot > targetTop || top < targetBot || left > targetRight || right < targetLeft);
	if (collides){
		if ((entityType == ENTITY_ENEMY && entity->entityType == ENTITY_PLAYER))
		{
			entity->isAlive = false;
		}
		else if ((entityType == ENTITY_PLAYER && entity->entityType == ENTITY_ENEMY))
		{
			isAlive = false;
		}
	}
	return collides;
	
}
void Entity::performCollision(Entity *entity)
{
	float targetRight = entity->x + entity->getWidth() * 0.5f;
	float targetLeft = entity->x - entity->getWidth() * 0.5f;
	float targetTop = entity->y + entity->getHeight() * 0.5f;
	float targetBot = entity->y - entity->getHeight() * 0.5f;

	float right = x + getWidth() * 0.5f;
	float left = x - getWidth() * 0.5f;
	float top = y + getHeight() * 0.5f;
	float bot = y - getHeight() * 0.5f;


	bool collides = !(bot > targetTop || top < targetBot || left > targetRight || right < targetLeft);
	//if (collides && entity->isAlive && isAlive){
	//	/*if ((entityType == ENTITY_ENEMY && entity->entityType == ENTITY_PLAYER))
	//	{
	//		entity->isAlive = false;
	//	}
	//	else if ((entityType == ENTITY_PLAYER && entity->entityType == ENTITY_ENEMY))
	//	{
	//		isAlive = false;
	//	}*/

	//	if ((entityType == ENTITY_PLAYER && entity->entityType == ENTITY_ENEMY) && bot >= (entity->y + entity->getHeight() * 0.45f) )
	//	{
	//		entity->isAlive = false;
	//	}
	//	else if ((entityType == ENTITY_PLAYER && entity->entityType == ENTITY_ENEMY) && bot < entity->y - entity->getHeight() *0.49f && entity->isAlive)
	//	{
	//		isAlive = false;
	//	}

	//}
	bool stomps = (bot < targetTop );
	if ((entityType == ENTITY_PLAYER && entity->entityType == ENTITY_ENEMY) && stomps && collides && actionState == ACTION_JUMPING)
	{
		entity->isAlive = false;
	}
	else if ((entityType == ENTITY_PLAYER && entity->entityType == ENTITY_ENEMY)  && collides)
	{
		isAlive = false;
	}
}
void Entity::update(float elapsed, float friction_x, float gravity_y)
{
	if (!isStatic && isAlive)
	{
		velocity_x = lerp(velocity_x, 0.0f, elapsed * friction_x);
		velocity_y += gravity_y * elapsed;

		velocity_x += acceleration_x * elapsed;
		velocity_y += acceleration_y * elapsed;
		if (collidedBottom) { velocity_y = 0.0f; actionState = ACTION_IDLE; }
		if (collidedRight || collidedLeft) velocity_x = 0.0f;
		x += velocity_x * elapsed;
		y += velocity_y * elapsed;
	}
	else if (!isStatic && !isAlive)
	{
		x = 0.1f;
		y = 0.1f;
		velocity_x = 0.0f;
		velocity_y = 0.0f;
	}
}