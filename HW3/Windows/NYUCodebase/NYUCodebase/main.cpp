//CS3113 HW #3
//William He
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <algorithm>
#include <math.h>
#include <vector>
#include "Entity.h"
#include "Bullet.h"
#include "ShaderProgram.h"
#include "Matrix.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
#define MAX_BULLETS 30
#define BULLET_VELOCITY 1.5f
#define FIXED_TIMESTEP 1.0f/60.0f
#define MAX_TIMESTEPS 6


SDL_Window* displayWindow;
bool done = false;
SDL_Event event;
Matrix projectionMatrix, modelMatrix, viewMatrix;
const Uint8 *keys = SDL_GetKeyboardState(NULL);
float lastFrameTicks = 0.0f;
Entity *Player = new Entity(0.0f, -0.7f, 0.0f, 2, 0.25f, 0.25f, 0.50f, 0.0f, 0.0f, true, PLAYER);
Entity* aliens[55];
int bulletIndex = 0;
int enemyBulletIndex = 0;
Bullet bullets[MAX_BULLETS];
Bullet enemyBullets[MAX_BULLETS];
GLuint fontSheet = 0;
ShaderProgram* program;
enum GameState { STATE_TITLE_SCREEN, STATE_GAME};
GameState currentState = STATE_TITLE_SCREEN;
bool gameOver = false;


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
void shootBullet(Entity* bulletSource) {
	if (bulletSource->role == PLAYER){
		if ((!bullets[bulletIndex - 1].isAlive &&  bullets[bulletIndex - 1].timeAlive > 1.2f) || bulletIndex == 0){
			bullets[bulletIndex].x = bulletSource->x;
			bullets[bulletIndex].y = bulletSource->y + bulletSource->height*0.5f;
			bullets[bulletIndex].speed = BULLET_VELOCITY;
			bullets[bulletIndex].direction_y = 1.0f;
			bullets[bulletIndex].isAlive = true;
			bullets[bulletIndex].timeAlive = 0.0f;
			bulletIndex++;
		}
		if (bulletIndex > MAX_BULLETS - 1) {
			bulletIndex = 0;
		}
	}
	else if (bulletSource->role == ENEMY && bulletSource->isAlive){
		if ((!enemyBullets[enemyBulletIndex - 1].isAlive &&  enemyBullets[enemyBulletIndex - 1].timeAlive > 3.5f) || enemyBulletIndex == 0){
			enemyBullets[enemyBulletIndex].x = bulletSource->x;
			enemyBullets[enemyBulletIndex].y = bulletSource->y - bulletSource->height*0.5f;
			enemyBullets[enemyBulletIndex].speed = 1.0f;
			enemyBullets[enemyBulletIndex].direction_y = -1.0f;
			enemyBullets[enemyBulletIndex].isAlive = true;
			enemyBullets[enemyBulletIndex].timeAlive = 0.0f;
			enemyBulletIndex++;
		}
		if (enemyBulletIndex > MAX_BULLETS - 1) {
			enemyBulletIndex = 0;
		}
	}
}
void DrawText(ShaderProgram *program, int fontTexture, std::string text, float size, float spacing) {
	float texture_size = 1.0 / 16.0f;
	std::vector<float> vertexData;
	std::vector<float> texCoordData;
	for (int i = 0; i < text.size(); i++) {
		float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
		float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
		vertexData.insert(vertexData.end(), {
			((size + spacing) * i) + (-0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
		});
		texCoordData.insert(texCoordData.end(), {
			texture_x, texture_y,
			texture_x, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x + texture_size, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x, texture_y + texture_size,
		});
	}
	glUseProgram(program->programID);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

void setup(){
	//SDL
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("William He - CS3113 HW#3 Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 853, 480, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	//OpenGL
	glViewport(0, 0, 853, 480);

	//Projection Matrix
	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	program = new ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	Player->textureID = loadTexture(RESOURCE_FOLDER"flat_medal1.png");
	GLuint spriteSheet = loadTexture(RESOURCE_FOLDER"characters_3.png");
	fontSheet = loadTexture(RESOURCE_FOLDER"font1.png");
	glUseProgram(program->programID);
	
	for (int i = 0; i < 55; i++)
	{
		int row = i / 11;
		int col = i % 11;
		aliens[i] = new Entity(-0.7f + 0.17f*col, 0.8f - 0.15f*row, 0.0f, spriteSheet, 0.20f, 0.20f, 0.10f, 1.0f, 0.0f);
	}
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		bullets[i] = Bullet();
		enemyBullets[i] = Bullet();
	}
	
}
void processEvents(){
	//SDL event loop
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
		else if (event.type == SDL_KEYDOWN && currentState == STATE_GAME){
			if (event.key.keysym.scancode == SDL_SCANCODE_SPACE)
			{
				shootBullet(Player);
				
			}
		}
	}
}

void update(float elapsed){
	if (currentState == STATE_TITLE_SCREEN)
	{
		if (keys[SDL_SCANCODE_RETURN])
			currentState = STATE_GAME;
	}
	else if (currentState == STATE_GAME)
	{
		//Player movements
		if (keys[SDL_SCANCODE_LEFT]){
			Player->direction_x = -1.0f;
			Player->x += Player->direction_x * elapsed * Player->speed;
		}
		else if (keys[SDL_SCANCODE_RIGHT]){
			Player->direction_x = 1.0f;
			Player->x += Player->direction_x * elapsed * Player->speed;
		}
		if (Player->x < -1.77f + Player->width*0.5f)
			Player->x = -1.77f + Player->width*0.5f;
		else if (Player->x >1.77f - Player->width *0.5f)
			Player->x = 1.77f - Player->width*0.5f;
		//Bullets
		for (int i = 0; i < MAX_BULLETS; i++)
		{
			bullets[i].update(elapsed);
			enemyBullets[i].update(elapsed);
		}
		for (int i = 0; i < MAX_BULLETS; i++)
		{
			enemyBullets[i].colliding(Player);
		}

		float aliensMaxPosition = 0.0f;
		float aliensMinPosition = 0.0f;
		int random = std::rand() % 55;

		while (random < 44 && aliens[random + 11]->isAlive){
			random += 11;
		}
		shootBullet(aliens[random]);

		bool outOfScreen = false;
		bool belowPlayer = false;
		for (int i = 0; i < 55; i++){
			if (aliens[i]->isAlive){
				for (int k = 0; k < MAX_BULLETS; k++)
				{
					bullets[k].colliding(aliens[i]);
				}
				if (aliens[i]->isAlive && (aliens[i]->x <-1.77f + aliens[i]->width*0.5f ))
				{
					aliens[i]->x = -1.77f + aliens[i]->width*0.5f;
					outOfScreen = true;
				}
				if (aliens[i]->isAlive && aliens[i]->x > 1.77f - aliens[i]->width *0.5f)
				{
					aliens[i]->x = 1.77f - aliens[i]->width*0.5f;
					outOfScreen = true;
				}
				if (aliens[i]->isAlive && (aliens[i]->y - aliens[i]->height*0.5f < Player->y))
				{
					gameOver = true;
					currentState = STATE_TITLE_SCREEN;
				}
			
			}
		}
		for (int i = 0; i < 55; i++){
			if (aliens[i]->isAlive){
				if (outOfScreen){
					aliens[i]->direction_x *= -1.0f;
					aliens[i]->y += -0.10f;
				}
				aliens[i]->update(elapsed);

			}
		}
	}
}

//move stuff and check for collisions
void update(){
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	float elapsed = ticks - lastFrameTicks;
	lastFrameTicks = ticks;

	float fixedElapsed = elapsed;
	if (fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS) {
		fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
	}
	while (fixedElapsed >= FIXED_TIMESTEP) {
		fixedElapsed -= FIXED_TIMESTEP;
		update(FIXED_TIMESTEP);
	}
	update(fixedElapsed);

}
void render(){
	program->setModelMatrix(modelMatrix);
	program->setProjectionMatrix(projectionMatrix);
	program->setViewMatrix(viewMatrix);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(44/255.0f, 62/255.0f, 80/255.0f, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	if (currentState == STATE_TITLE_SCREEN)
	{
		if (gameOver)
		{
			Player->isAlive = true;
			for (int i = 0; i < 55; i++)
			{
				int row = i / 11;
				int col = i % 11;
				aliens[i]->isAlive = true;
				aliens[i]->x = -0.7f + 0.17f*col;
				aliens[i]->y = 0.8f - 0.15f*row;
				aliens[i]->direction_x = 1.0f;

			}
			gameOver = false;
		}
		modelMatrix.identity();
		modelMatrix.Translate(-1.2f, 1.5f, 0.0f);
		program->setModelMatrix(modelMatrix);
		DrawText(program, fontSheet, "Space Invaders", 0.33f, -0.1f);
		modelMatrix.identity();
		modelMatrix.Translate(-1.67f, 0.0f, 0.0f);
		program->setModelMatrix(modelMatrix);
		DrawText(program, fontSheet, "Press enter to start", 0.2f, 0.0f);
		modelMatrix.identity();
		modelMatrix.Translate(-0.50f, -0.5f, 0.0f);
		program->setModelMatrix(modelMatrix);
		DrawText(program, fontSheet, "Space = shoot ", 0.17f, -0.07f);
		modelMatrix.identity();
		modelMatrix.Translate(-0.57f, -0.7f, 0.0f);
		program->setModelMatrix(modelMatrix);
		DrawText(program, fontSheet, "Left/Right = move ", 0.17f, -0.07f);
	}
	else if (currentState == STATE_GAME)
	{
		modelMatrix.identity();
		program->setModelMatrix(modelMatrix);

		for (int i = 0; i < 55; i++)
		{
			int row = i / 11;
			if (aliens[i]->isAlive)
			{
				modelMatrix.identity();
				modelMatrix.Translate(aliens[i]->x, aliens[i]->y, 0.0f);
				program->setModelMatrix(modelMatrix);
				if (row == 0 || row == 1)
				{
					aliens[i]->DrawSpriteSheetSprite(program, 8, 8, 4);
				}
				if (row == 2 || row == 3)
				{
					aliens[i]->DrawSpriteSheetSprite(program, 0, 8, 4);

				}
				if (row == 4)
				{
					aliens[i]->DrawSpriteSheetSprite(program, 16, 8, 4);

				}
			}
		}
		if (Player->isAlive){
			modelMatrix.identity();
			modelMatrix.Translate(Player->x, Player->y, 0.0f);
			program->setModelMatrix(modelMatrix);
			Player->Draw(program);
		}
		else if (!Player->isAlive)
		{
			gameOver = true;
		
			currentState = STATE_TITLE_SCREEN;
			return;
		}
		for (int i = 0; i < MAX_BULLETS; i++)
		{
			modelMatrix.identity();
			modelMatrix.Translate(bullets[i].x, bullets[i].y, 0.0f);
			program->setModelMatrix(modelMatrix);

			if (bullets[i].isAlive)
				bullets[i].Draw(program);
			modelMatrix.identity();
			modelMatrix.Translate(enemyBullets[i].x, enemyBullets[i].y, 0.0f);
			program->setModelMatrix(modelMatrix);

			if (enemyBullets[i].isAlive)
				enemyBullets[i].Draw(program);
		}
	}
	SDL_GL_SwapWindow(displayWindow);
	

}


void cleanUp(){
	SDL_Quit();
	delete Player;
	for (int i = 0; i < 55; i++)
	{
		delete aliens[i];
	}
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
