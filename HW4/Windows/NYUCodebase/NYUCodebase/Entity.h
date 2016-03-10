#pragma once

#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"

class Entity
{
public: 
	Entity(float _x = 0.0f, float _y = 0.0f, float _rotation = 0.0f, int _textureID = 0, float _width = 0.1f, float _height = 1.0f, float _speed = 0.0f, float _direction_x = 0.0f, float _direction_y = 0.0f);
	void Draw(ShaderProgram *program);
	float x;
	float y;
	float rotation;
	int textureID;
	float width;
	float height;
	float speed;
	float direction_x;
	float direction_y;

	
};