#pragma once

#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "SpriteSheet.h"

enum EntityType
{
	PLAYER, ENEMY
};
class Entity
{
public: 
	//Entity();
	Entity(float _x = -2000.0f, float _y = -2000.0f, float _rotation = 0.0f, int _textureID = 0, float _width = 0.010f, float _height = 0.1f, float _speed = 0.0f, float _direction_x = 0.0f, float _direction_y = 0.0f, bool _isAlive = true, EntityType _role=ENEMY);
	//Entity(float x, float y, float rotation, SheetSprite sprite);
	void Draw(ShaderProgram *program);
	void update(float elapsed);
	void DrawSpriteSheetSprite(ShaderProgram *program, int index, int spriteCountX,
		int spriteCountY);
	float x;
	float y;
	float rotation;
	int textureID;
	float width;
	float height;
	float speed;
	float direction_x;
	float direction_y;
	bool isAlive;
	EntityType role;
	//SheetSprite sprite;
};