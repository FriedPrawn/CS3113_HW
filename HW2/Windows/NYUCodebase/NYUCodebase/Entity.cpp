#include "Entity.h"

Entity::Entity(float _x, float _y, float _rotation, int _textureID, float _width, float _height , float _speed, float _direction_x, float _direction_y)
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
		0.0f, 1.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 1.0f
	};

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->texCoordAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);

}

