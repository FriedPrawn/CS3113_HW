#include "Player.h"

Player::Player(): hitpoints(1), isAlive(true){}
void Player::update(float elapsed, float friction_x, float gravity_y)
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
