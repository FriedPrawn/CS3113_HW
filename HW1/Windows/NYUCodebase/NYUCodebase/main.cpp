//CS3113 HW #1
//William He
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#include <math.h>
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

GLuint loadTexture(const char* imagePath)
{
	SDL_Surface *surface = IMG_Load(imagePath);
	
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
//setting the texture data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	SDL_FreeSurface(surface);

	return textureID;
}


int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("William He - CS3113 HW#1 Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	//openGL setup:
	glViewport(0, 0, 640, 360);
	//shader needs to support texture
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	GLuint shipTexture = loadTexture(RESOURCE_FOLDER "playerShip1_blue.png");
	GLuint bushTexture = loadTexture(RESOURCE_FOLDER "bush.png");
	GLuint shroomTexture = loadTexture(RESOURCE_FOLDER "shroomBrownAltMid.png");
	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;

	//set matrixes 3.55:2 is 16:9 ratio
	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(program.programID);

	//time tracking
	float lastFrameTicks = 0.0f;
	
	float shroomAngle = 0.0f;
	float shipPosition = 0.0f;
	float bushPosition = 0.0f;

//program loop
	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

//Movement
		shipPosition += 0.2f * elapsed;
		bushPosition -= 0.5f * elapsed;
		shroomAngle += 0.01f * (3.141592653f / 180);
//Drawing polygons
		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);
		

		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glVertexAttribPointer(index, size, type, normalized, stride, const pointer)
		
		

//Display triangle after clear	
		
		
		glClearColor(0.4f, 0.2f, 0.4f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	
	//Ship sprite
		if (shipPosition > 2.0)
			shipPosition = 0;
		modelMatrix.identity();
		modelMatrix.Translate(0.0f, shipPosition, 0.0f);
		program.setModelMatrix(modelMatrix);

		glBindTexture(GL_TEXTURE_2D, shipTexture);
		float shipVertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, shipVertices);
		glEnableVertexAttribArray(program.positionAttribute);

		//2 triangles diagonal from top right to bottom left
		float shipTexCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, shipTexCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);

	//Bush sprite
		if (bushPosition < -3.55)
			bushPosition = 0;
		modelMatrix.identity();
		modelMatrix.Translate(bushPosition, 0.0f, 0.0f);
		program.setModelMatrix(modelMatrix);

		glBindTexture(GL_TEXTURE_2D, bushTexture);
		float bushVertices[] = { -0.75,-0.75, 0.0,-0.75, 0.0,0.0, -0.75,-0.75, 0.0, 0.0, -0.75,0.0  };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, bushVertices);
		glEnableVertexAttribArray(program.positionAttribute);

		float bushTexCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, bushTexCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);

	//Shroom sprite
		modelMatrix.identity();
		modelMatrix.Rotate(shroomAngle);
		program.setModelMatrix(modelMatrix);

		glBindTexture(GL_TEXTURE_2D, shroomTexture);
		float shroomVertices[] = { 1.0,1.0, 1.5,1.0, 1.5, 1.5, 1.0,1.0, 1.5,1.5, 1.0,1.5 };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, shroomVertices);
		glEnableVertexAttribArray(program.positionAttribute);

		float shroomTexCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, shroomTexCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
