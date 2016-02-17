//CS3113 HW #2
//William He
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Entity.h"
#include "ShaderProgram.h"
#include "Matrix.h"
#include <math.h>
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;
bool done = false;
SDL_Event event;
Matrix projectionMatrix, modelMatrix, viewMatrix;
const Uint8 *keys = SDL_GetKeyboardState(NULL);
float lastFrameTicks = 0.0f;
Entity *Player1 = new Entity(-1.67f, 0.0f, 0.0f, -1, 0.1, 1.0f, 1.0f, 0, 0);
Entity *Player2 = new Entity(1.67f, 0.0f, 0.0f, -1, 0.1f, 1.0f, 1.0f, 0, 0);
Entity *Ball = new Entity(0.0f, 0.0f, 0.0f, -1, 0.1f, 0.1f, 1.0f, 1.0f, 1.0f);
Entity *Winner = new Entity(1.0f, 0.7f, 0.0f, -1, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f);

bool win1 = false;
bool win2 = false;
float winShow = 0.0f;
ShaderProgram* program;

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
GLuint winnerTexture = loadTexture(RESOURCE_FOLDER"bush.png");

void setup(){
	//SDL
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("William He - CS3113 HW#2 Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	//OpenGL
	glViewport(0, 0, 640, 360);

	//Projection Matrix
	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	program = new ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");


	glUseProgram(program->programID);


}
void processEvents(){
	//SDL event loop

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
	}
}


//move stuff and check for collisions
void update(){
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	float elapsed = ticks - lastFrameTicks;
	lastFrameTicks = ticks;
	
	if (Ball->x > 1.77f)
	{
		win1 = true;
		winShow += elapsed;
	}
	if (Ball->x < -1.77f)
	{
		win2 = true;
		winShow += elapsed;

	}
	if (winShow > 1.0f)
	{
		winShow = 0.0f;
		win1 = false;
		win2 = false;
		Ball->x = 0.0f;
		Ball->y = 0.0f;
	}
	if (keys[SDL_SCANCODE_W]){
		Player1->direction_y = 1.0f;
		Player1->y += Player1->direction_y * elapsed * Player1->speed;
	}
	else if (keys[SDL_SCANCODE_S]){
		Player1->direction_y = -1.0f;	
		Player1->y += Player1->direction_y * elapsed * Player1->speed;
	}
	if (keys[SDL_SCANCODE_UP]){
		Player2->direction_y = 1.0f;
		Player2->y += Player2->direction_y * elapsed * Player2->speed;

	}
	else if (keys[SDL_SCANCODE_DOWN]){
		Player2->direction_y = -1.0f;
		Player2->y += Player2->direction_y * elapsed * Player2->speed;

	}
	
	if (Player1->y > 0.8f)
		Player1->y = 0.8f;
	else if (Player1->y < -0.8f)
		Player1->y = -0.8f;
	if (Player2->y > 0.8f)
		Player2->y = 0.8f;
	else if (Player2->y < -0.8f)
		Player2->y = -0.8f;
	

	if (
		((Ball->x - (Ball->width / 2.0f) >(Player2->x - ((Player2->width) / 2.0f)))
		&&(Ball->y < (Player2->y + (0.25f * Player2->height)  ))
		&& (Ball->y > (Player2->y - (0.25f * Player2->height)))
		)
		|| (Ball->x + (Ball->width/2.0f) < (Player1->x + (Player1->width/2.0f))
		&& (Ball->y < (Player1->y + (0.25f * Player1->height)))
		&& (Ball->y >(Player1->y - (0.25f * Player1->height)))
		)
		)
	{
		Ball->direction_x *= -1.0f;

	}
	if (Ball->y <= -1.0f || Ball->y >= 1.0f){
		Ball->direction_y *= -1.0f;
	}
	Ball->x += Ball->direction_x * elapsed * Ball->speed;
	Ball->y += Ball->direction_y * elapsed * Ball->speed;


}
void render(){
	program->setModelMatrix(modelMatrix);
	program->setProjectionMatrix(projectionMatrix);
	program->setViewMatrix(viewMatrix);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.4f, 0.2f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	if (!win1 && !win2)
	{
		modelMatrix.identity();
		modelMatrix.Translate(Player1->x, Player1->y, 0.0f);
		program->setModelMatrix(modelMatrix);
		Player1->Draw(program);

		modelMatrix.identity();
		modelMatrix.Translate(Player2->x, Player2->y, 0.0f);
		program->setModelMatrix(modelMatrix);
		Player2->Draw(program);

		modelMatrix.identity();
		modelMatrix.Translate(Ball->x, Ball->y, 0.0f);
		program->setModelMatrix(modelMatrix);
		Ball->Draw(program);
	}
	else if (win1 || win2){
		if (win1){
			Ball->x = 2.0f;
			Winner->x = -1.0f;
			modelMatrix.identity();
			modelMatrix.Translate(Winner->x, Winner->y, 0.0f);
			glBindTexture(GL_TEXTURE_2D, winnerTexture);
			program->setModelMatrix(modelMatrix);
			Winner->Draw(program);


		}
		else if (win2){
			Winner->x = 1.0f;
			Ball->x = -2.0f;
			glBindTexture(GL_TEXTURE_2D, winnerTexture);
			modelMatrix.identity();
			modelMatrix.Translate(Winner->x, Winner->y, 0.0f);
			program->setModelMatrix(modelMatrix);
			Winner->Draw(program);

		}

	}

	SDL_GL_SwapWindow(displayWindow);
	

}


void cleanUp(){
	SDL_Quit();
	delete Player1, Player2, Ball;
}



int main(int argc, char *argv[])
{

	setup();
	while (!done) {
		processEvents();
		update();
		render();

	}
	cleanUp();
	return 0;
}
