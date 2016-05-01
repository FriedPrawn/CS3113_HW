#include "Entity.h"

//Entity::Entity(SheetSprite sprite, bool isStatic, float velocity_x, float velocity_y, float acceleration_x, float acceleration_y, float x, float y, float width, float height) :
//	isStatic(isStatic),
//	velocity_x(velocity_x),
//	velocity_y(velocity_y),
//	acceleration_x(acceleration_x),
//	acceleration_y(acceleration_y),
//	x(x), 
//	y(y), 
//	width(width), 
//	height(height), 
//	sprite(sprite){}

Entity::Entity(SheetSprite sprite, bool isStatic, float x, float y, float velocity_x, float velocity_y, float acceleration_x, float acceleration_y) :
	sprite(sprite),
	isStatic(isStatic),
	velocity_x(velocity_x),
	velocity_y(velocity_y),
	acceleration_x(acceleration_x),
	acceleration_y(acceleration_y),
	x(x),
	y(y)
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

	//if (collides)
	//{
	//	if (!isStatic)
	//	{
	//		if (bot < targetTop )
	//			collidedBottom = true;
	//		else if (top > targetBot)
	//			collidedTop = true;
	//		if (left < targetRight)
	//			collidedLeft = true;
	//		else if (right > targetLeft)
	//			collidedRight = true;
	//	}
	//}
	return collides;
	
}
void Entity::update(float elapsed, float friction_x, float gravity_y)
{
	if (!isStatic)
	{
		velocity_x = lerp(velocity_x, 0.0f, elapsed * friction_x);
		velocity_y += gravity_y * elapsed;

		velocity_x += acceleration_x * elapsed;
		velocity_y += acceleration_y * elapsed;
		if (collidedBottom) velocity_y = 0.0f;
		if (collidedRight || collidedLeft) velocity_x = 0.0f;
		x += velocity_x * elapsed;
		y += velocity_y * elapsed;
	}

}