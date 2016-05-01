#pragma once

#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "SheetSprite.h"

enum EntityType
{
	ENTITY_PLAYER, ENTITY_ENEMY, ENTITY_COIN
};
class Entity
{
public: 
	//Entity(SheetSprite sprite = SheetSprite(), bool isStatic = false, float velocity_x = 0.0f, float velocity_y = 0.0f, float acceleration_x = 0.0f, float acceleration_y = 0.0f, float x = 0.0f, float y = 0.0f, float width = 0.25f, float height = 0.25f);
	Entity(SheetSprite sprite = SheetSprite(), bool isStatic = false, float x = 0.0f, float y = 0.0f, float velocity_x = 0.0f, float velocity_y = 0.0f, float acceleration_x = 0.0f, float acceleration_y = 0.0f );
	void update(float elapsed, float friction_x = 0.0f, float gravity_y = 0.0f);
	void Render(ShaderProgram *program);
	bool collidesWith(Entity *entity);
	float lerp(float v0, float v1, float t);
	float getWidth();
	float getHeight();
	SheetSprite sprite;
	float x;
	float y;
	//float width;
	//float height;
	float velocity_x;
	float velocity_y;
	float acceleration_x;
	float acceleration_y;

	bool isStatic;
	EntityType entityType;

	bool collidedTop = false;
	bool collidedBottom = false;
	bool collidedLeft = false;
	bool collidedRight = false;
};