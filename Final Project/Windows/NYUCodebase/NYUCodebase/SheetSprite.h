#pragma once
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"

class SheetSprite {
public:
	SheetSprite();
	SheetSprite(unsigned int textureID);
	SheetSprite(unsigned int textureID, float u, float v, float width, float height, float
		size);
	void Draw(ShaderProgram* program, Matrix &modelMatrix, float x, float y);
	
	unsigned int textureID;
	float size;
	float u;
	float v;
	float width;
	float height;
};