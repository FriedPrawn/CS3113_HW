#include "SheetSprite.h"

SheetSprite::SheetSprite() : textureID(0), u(0.0f), v(0.0f), width(1.0f), height(1.0f), size(1.0f)
{};
SheetSprite::SheetSprite(unsigned int textureID) : textureID(textureID), u(0.0f), v(0.0f), width(1.0f), height(1.0f), size(0.2f){}

SheetSprite::SheetSprite(unsigned int textureID, float u, float v, float width, float height, float
	size) : textureID(textureID), u(u), v(v), width(width), height(height), size(size){}


void SheetSprite::Draw(ShaderProgram* program, Matrix &modelMatrix, float x, float y) {


	GLfloat texCoords[] = {
		u, v + height,
		u + width, v,
		u, v,
		u + width, v,
		u, v + height,
		u + width, v + height
	};
	float aspect = width / height;
	float vertices[] = {
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, 0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, -0.5f * size };

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);
	//glBindTexture(GL_TEXTURE_2D, textureID);
	
	modelMatrix.identity();
	modelMatrix.Translate(x, y, 0.0f);
	program->setModelMatrix(modelMatrix);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

void SheetSprite::DrawUniformSheet(ShaderProgram *program, Matrix &modelMatrix, float x, float y, int index, int spriteCountX,
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
	float aspect = width / height;
	float vertices[] = {
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, 0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, -0.5f * size };
	// our regular sprite drawing

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);
	//glBindTexture(GL_TEXTURE_2D, textureID);

	modelMatrix.identity();
	modelMatrix.Translate(x, y, 0.0f);
	program->setModelMatrix(modelMatrix);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}