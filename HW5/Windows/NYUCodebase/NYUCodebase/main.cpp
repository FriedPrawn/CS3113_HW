//CS3113 HW #5
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
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include "Entity.h"
#include "ShaderProgram.h"
#include "Matrix.h"
using namespace std;

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
#define FRICTION 1.5f
#define GRAVITY -5.5f
#define PENETRATION 0.0000000000001f
#define FIXED_TIMESTEP 1.0f/60.0f
#define MAX_TIMESTEPS 6

SDL_Window* displayWindow;
bool done = false;
SDL_Event event;
Matrix projectionMatrix, modelMatrix, viewMatrix;
const Uint8 *keys = SDL_GetKeyboardState(NULL);
float lastFrameTicks = 0.0f;
Entity Player;
Entity* tiles[10];
GLuint fontSheet = 0;
ShaderProgram* program;
enum GameState { STATE_TITLE_SCREEN, STATE_GAME};
GameState currentState = STATE_TITLE_SCREEN;
bool gameOver = false;
int mapWidth;
int mapHeight;
unsigned char** levelData;
float TILE_SIZE = 0.5f;
int * currGridX = new int;
int * currGridY = new int;
GLuint tileset;
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
bool readHeader(std::ifstream &stream) {
	string line;
	mapWidth = -1;
	mapHeight = -1;
	while (getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "width") {
			mapWidth = atoi(value.c_str());
		}
		else if (key == "height"){
			mapHeight = atoi(value.c_str());
		}
	}
	if (mapWidth == -1 || mapHeight == -1) {
		return false;
	}
	else { // allocate our map data
		levelData = new unsigned char*[mapHeight];
		for (int i = 0; i < mapHeight; ++i) {
			levelData[i] = new unsigned char[mapWidth];
		}
		return true;
	}
}
bool readLayerData(std::ifstream &stream) {
	string line;
	while (getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "data") {
			for (int y = 0; y < mapHeight; y++) {
				getline(stream, line);
				istringstream lineStream(line);
				string tile;
				for (int x = 0; x < mapWidth; x++) {
					getline(lineStream, tile, ',');
					unsigned char val = (unsigned char)atoi(tile.c_str());
					if (val > 0) {
						// be careful, the tiles in this format are indexed from 1 not 0
						levelData[y][x] = val - 1;
						//if (val == 1 || val == 0 || val == 21 || val == 35)
							//brickTiles.push_back(val);
					}
					else {
						levelData[y][x] = 0;
					}
				}
			}
		}
	}
	return true;
}
void loadLevel()
{
	tileset = loadTexture(RESOURCE_FOLDER"arne_sprites.png");
	ifstream infile("gamemap.txt");
	string line;
	while (getline(infile, line)){
		if (line == "[header]")
		{
			if (!readHeader(infile)){
				return;
			}
		}
		else if (line == "[layer]"){
			readLayerData(infile);
		}
	}
	infile.close();

}

void renderLevel()
{
	std::vector<float> vertexData;
	std::vector<float> texCoordData;
	int SPRITE_COUNT_X = 16;
	int SPRITE_COUNT_Y = 8;
	for (int y = 0; y < mapHeight; y++) {
		for (int x = 0; x < mapWidth; x++) {
			float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
			float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
			float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
			float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
			vertexData.insert(vertexData.end(), {
				TILE_SIZE * x, -TILE_SIZE * y,
				TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
				(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
				TILE_SIZE * x, -TILE_SIZE * y,
				(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
				(TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y
			});
			texCoordData.insert(texCoordData.end(), {
				u, v,
				u, v + (spriteHeight),
				u + spriteWidth, v + (spriteHeight),
				u, v,
				u + spriteWidth, v + (spriteHeight),
				u + spriteWidth, v
			});
		}
	}

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);
	//glBindTexture(GL_TEXTURE_2D, textureID);

	modelMatrix.identity();
	program->setModelMatrix(modelMatrix);
	glBindTexture(GL_TEXTURE_2D, tileset);
	glDrawArrays(GL_TRIANGLES, 0, 6*mapHeight*mapWidth);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY) {
	*gridX = (int)(worldX / TILE_SIZE);
	*gridY = (int)(-worldY / TILE_SIZE);
}

void setup(){
	//SDL
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("William He - CS3113 HW#5 Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 853, 480, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 853, 480);
	projectionMatrix.setOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
	program = new ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	GLuint oreoTexture = loadTexture(RESOURCE_FOLDER"cookieBrown.png");
	GLuint spriteSheetTexture = loadTexture(RESOURCE_FOLDER"aliens.png");
	fontSheet = loadTexture(RESOURCE_FOLDER"font1.png");
	glUseProgram(program->programID);
	
	for (int i = 0; i < 10; i++)
	{
		tiles[i] = new Entity(SheetSprite(oreoTexture, 0.0f, 0.0f, 1.0f, 1.0F, 0.6f), true, -2.7f + 1.5f*i, -0.8f);
	}	
	Player =  Entity(SheetSprite(spriteSheetTexture, 676.0f / 1024.0f, 184.0f / 512.0f, 66.0f / 1024.0f, 92.0f /
		512.0f, 0.6f),false, 2.0f, -1.0f);
	loadLevel();
}
void processEvents(){
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
		else if (event.type == SDL_KEYDOWN){
			if (event.key.keysym.scancode == SDL_SCANCODE_SPACE && Player.collidedBottom){
				Player.velocity_y = 5.0f;
			}
		}
	}
}

void update(float elapsed){

	if (keys[SDL_SCANCODE_LEFT] && !Player.collidedLeft){
		Player.acceleration_x = -4.0f;
	}
	else if (keys[SDL_SCANCODE_RIGHT] && !Player.collidedRight){
		Player.acceleration_x = 4.0f;
	}
	else{
		Player.acceleration_x = 0.0f;
	}
	Player.collidedBottom = false;
	Player.collidedTop = false;
	Player.collidedLeft = false;
	Player.collidedRight = false;
	int currX1, currY1, currX2, currY2;
	float bot = Player.y - Player.getHeight() * 0.5f;
	float top = Player.y + Player.getHeight() * 0.5f;
	float left = Player.x - Player.getWidth() * 0.5f;
	float right = Player.x + Player.getWidth() * 0.5f;
	float penetration = 0.0f;
//Penetrate Bot
	worldToTileCoordinates(Player.x - Player.getWidth() / 2, Player.y - Player.getHeight() / 2, &currX1, &currY1);
	worldToTileCoordinates(Player.x + Player.getWidth() / 2, Player.y - Player.getHeight() / 2, &currX2, &currY2);
	if (levelData[currY1][currX1] == 0 || levelData[currY2][currX2] == 0 || levelData[currY1][currX1] == 19 || levelData[currY2][currX2] == 19)
	{
		penetration = fabs((-TILE_SIZE*(currY1)) - bot);
		Player.y += penetration + 0.000009f;
		Player.collidedBottom = true;
	}
//Penetrate Top
	worldToTileCoordinates(Player.x - Player.getWidth() / 2, Player.y + Player.getHeight() / 2, &currX1, &currY1);
	worldToTileCoordinates(Player.x + Player.getWidth() / 2, Player.y + Player.getHeight() / 2, &currX2, &currY2);
	if (levelData[currY1][currX1] == 0 || levelData[currY2][currX2] == 0 || levelData[currY1][currX1] == 19 || levelData[currY2][currX2] == 19)
	{
		penetration = fabs((-TILE_SIZE*(currY1) - TILE_SIZE) - top);
		Player.y -= penetration + 0.000009f;
		Player.collidedTop = true;
		Player.velocity_y = 0.0f;
	}
//Penetrate Left
	worldToTileCoordinates(Player.x - Player.getWidth() / 2, Player.y , &currX1, &currY1);
	worldToTileCoordinates(Player.x - Player.getWidth() / 2, Player.y , &currX2, &currY2);
	if (levelData[currY1][currX1] == 0 || levelData[currY2][currX2] == 0 || levelData[currY1][currX1] == 19 || levelData[currY2][currX2] == 19)
	{
		penetration = fabs((TILE_SIZE * currX1 + TILE_SIZE) - left);
		Player.x += penetration + 0.00039f;
		Player.collidedLeft = true;
		Player.velocity_x = 0.0f;
	}

//Penetrate Right
	worldToTileCoordinates(Player.x + Player.getWidth() / 2, Player.y, &currX1, &currY1);
	worldToTileCoordinates(Player.x + Player.getWidth() / 2, Player.y, &currX2, &currY2);
	if (levelData[currY1][currX1] == 0 || levelData[currY2][currX2] == 0 || levelData[currY1][currX1] == 19 || levelData[currY2][currX2] == 19)
	{
		penetration = fabs((TILE_SIZE * currX1) - right);
		Player.x -= penetration + 0.00039f;
		Player.collidedRight = true;
		Player.velocity_x = 0.0f;
	}




	//for (int i = 0; i < 10; i++){
	//	if (Player.collidesWith(tiles[i])){
	//		 penetration = fabs((fabs(Player.y - tiles[i]->y)) - (Player.getHeight() / 2) -
	//			(tiles[i]->getHeight() / 2));
	//		if (Player.y - Player.getHeight()*0.5f <= tiles[i]->y + tiles[i]->getHeight()*0.5f
	//			){
	//			Player.y += penetration + 0.00000009f;
	//			Player.collidedBottom = true;
	//		}
	//		else {
	//			Player.y -= penetration + 0.0001f;
	//			Player.collidedTop = true;
	//		}
	//	}
	//}
	Player.update(elapsed, FRICTION, GRAVITY);
	
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
	renderLevel();
	
	for (int i = 0; i < 10; i++)
	{
		tiles[i]->sprite.Draw(program, modelMatrix, tiles[i]->x, tiles[i]->y);
	}
	Player.sprite.Draw(program, modelMatrix, Player.x, Player.y);
	viewMatrix.identity();
	viewMatrix.Translate(-1*Player.x, -1*Player.y, 0.0f);
	program->setViewMatrix(viewMatrix);
	SDL_GL_SwapWindow(displayWindow);
}


void cleanUp(){
	SDL_Quit();

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
