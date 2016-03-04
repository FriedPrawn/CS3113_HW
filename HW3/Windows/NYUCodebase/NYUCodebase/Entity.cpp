#include "Entity.h"

Entity::Entity(float _x, float _y, float _rotation, int _textureID, float _width, float _height, float _speed, float _direction_x, float _direction_y, bool _isAlive, EntityType _role) : role(_role)
{
	x = _x;
	y = _y;
	rotation = _rotation;
	textureID = _textureID;
	width = _width;
	height = _height;
	speed = _speed;
	direction_x = _direction_x;
	direction_y = _direction_y;
	isAlive = _isAlive;
	role = _role;
}
//Entity::Entity(float x, float y, float rotation, SheetSprite sprite) : x(x), y(y), rotation(rotation), sprite(sprite)
//{
//
//}

void Entity::DrawSpriteSheetSprite(ShaderProgram *program, int index, int spriteCountX,
	int spriteCountY) {
	float u = (float)(((int)index) % spriteCountX) / (float)spriteCountX;
	float v = (float)(((int)index) / spriteCountX) / (float)spriteCountY;
	float spriteWidth = 1.0 / (float)spriteCountX;
	float spriteHeight = 1.0 / (float)spriteCountY;
	GLfloat texCoords[] = {
		u, v + spriteHeight,
		u + spriteWidth, v,
		u, v,
		u + spriteWidth, v,
		u, v + spriteHeight,
		u + spriteWidth, v + spriteHeight
	};

	float vertices[] = {
		x + (-0.5f * width), y + (-0.5f * height),
		x + (0.5f * width), y + (0.5f * height),
		x + (-0.5f * width), y + (0.5f * height),
		x + (0.5f * width), y + (0.5f * height),
		x + (-0.5f * width), y + (-0.5f * height),
		x + (0.5f * width), y + (-0.5f * height)
		
	 };
	// our regular sprite drawing

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}
void Entity::Draw(ShaderProgram *program){

	float vertices[] = {
		x + (-0.5f * width), y + (0.5f * height),
		x + (-0.5f * width), y + (-0.5f * height),
		x + (0.5f * width), y + (0.5f * height),
		x + (0.5f * width), y + (-0.5f * height),
		x + (0.5f * width), y + (0.5f * height),
		x + (-0.5f * width), y + (-0.5f * height)
		//-0.5, 0.5, -0.5, -0.5, 0.5, 0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5
	};
	float texture_coords[] = {
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 1.0f

	/*	0.0f, 1.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 1.0f*/
	};

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coords);
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);

}



void Entity::update(float elapsed)
{
	if (isAlive)
	{
		x += direction_x * elapsed * speed;
		y += direction_y * elapsed * speed;
	}
	//if (!isAlive)
	//{
	//	x = -2000.0f;
	//	y = -2000.0f;
	//	direction_y = 0.0f;
	//	direction_x = 0.0f;
	//	speed = 0.0f;
	//}
}