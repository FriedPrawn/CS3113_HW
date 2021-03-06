//CS3113 Final Project
//William He
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <algorithm>
#include <math.h>
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <iterator>
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
Entity Enemy, Enemy2;
Entity* tiles[10];
SheetSprite background;
GLuint fontSheet = 0;
ShaderProgram* program;
enum GameState { STATE_TITLE_SCREEN, STATE_GAME, STATE_LEVEL2, STATE_LEVEL3, STATE_GAME_OVER, STATE_GAME_WIN};
GameState currentState = STATE_TITLE_SCREEN;
GameState latestLevel = STATE_GAME;
bool gameOver = false;
int mapWidth;
int mapHeight;
int tileWidth;
int tileHeight;
unsigned char** levelData;
float TILE_SIZE = 0.4f;
int * currGridX = new int;
int * currGridY = new int;
GLuint tileset;
GLuint tileset2;
GLuint backgroudTexture;
GLuint enemyWormTexture;
float placeXD = 0.0f;
Mix_Music *music;
Mix_Chunk *jump;
bool firstTime = true;
float portalx;
float portaly;

//animation
const int runAnimation[] = { 0, 1, 2, 7, 8, 9, 3, 4, 11, 5 };
const int numFrames = 10;
float animationElapsed = 0.0f;
float framesPerSecond = 60.0f;
int currentIndex = 0;
vector<int> staticTilesIndex{9,140,66,89,50,66, 26, 15, 123, 91, 27, 134, 22};
vector<int> lavaTilesIndex{ 127, 7, 94};
vector<Entity*> enemiesOne;
vector<Entity*> enemiesTwo;
vector<Entity*> enemiesThree;

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
		else if (key == "tilewidth"){
			tileWidth = atoi(value.c_str());
		}
		else if (key == "tileheight"){
			tileHeight = atoi(value.c_str());
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
bool readEntityData(std::ifstream &stream) {
	string line;
	string type;
	float offsetX = mapWidth/2.0f * TILE_SIZE;
	float offsetY = mapHeight/2.0f * TILE_SIZE;
	while (getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "type") {
			type = value;
		}
		else if (key == "location") {
			istringstream lineStream(value);
			string xPosition, yPosition;
			getline(lineStream, xPosition, ',');
			getline(lineStream, yPosition, ',');
			float placeX = float (atoi(xPosition.c_str()) / tileWidth) * TILE_SIZE;
			float placeY = float (atoi(yPosition.c_str()) / tileHeight) * -TILE_SIZE;
			

			if (type == "Worm")
			{
				if (currentState == STATE_GAME){
					enemiesOne.push_back(new Entity(ENTITY_ENEMY, SheetSprite(enemyWormTexture), false, placeX, placeY, 0.0f, 0.0f, 1.0f));
					if (enemiesOne.size() == 2)
					{
						enemiesOne[1]->acceleration_x = 0.0f;
					}
				}
				else if (currentState == STATE_LEVEL2)
				{
					enemiesTwo.push_back(new Entity(ENTITY_ENEMY, SheetSprite(enemyWormTexture), false, placeX, placeY, 0.0f, 0.0f, 0.0f));
				
				}
				else if (currentState == STATE_LEVEL3)
				{
					enemiesThree.push_back(new Entity(ENTITY_ENEMY, SheetSprite(enemyWormTexture), false, placeX, placeY, 0.0f, 0.0f, 1.0f));
				}
			}
			if (type == "Start")
			{

				Player.x = placeX;
				Player.y = placeY;
			}
			if (type == "Portal")
			{
				portalx = placeX;
				portaly = placeY;
			}
	
		}
	}
	return true;
}
void loadLevel()
{
	tileset = loadTexture(RESOURCE_FOLDER"tiles_spritesheet1.png");
	//ifstream infile("alienlevel1.txt");
	ifstream infile("Level1a.txt");

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
		else if (line == "[ObjectsLayer]"){
			readEntityData(infile);
		}
	}
	infile.close();

}
void loadLevel2()
{
	tileset = loadTexture(RESOURCE_FOLDER"tiles_spritesheet1.png");
	//ifstream infile("alienlevel1.txt");
	ifstream infile("level2.txt");

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
		else if (line == "[ObjectsLayer]"){
			readEntityData(infile);
		}
	}
	infile.close();

}
void loadLevel3()
{
	tileset = loadTexture(RESOURCE_FOLDER"tiles_spritesheet1.png");
	//ifstream infile("alienlevel1.txt");
	ifstream infile("Level3.txt");

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
		else if (line == "[ObjectsLayer]"){
			readEntityData(infile);
		}
	}
	infile.close();

}
void renderLevel()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	std::vector<float> vertexData;
	std::vector<float> texCoordData;
	int SPRITE_COUNT_X = 11;
	int SPRITE_COUNT_Y = 13;
	for (int y = 0; y < mapHeight; y++) 
	{
		for (int x = 0; x < mapWidth; x++) 
		{
			
			float spacingX = 2.5f / 792.0f;
			float spacingY = 2.0f / 936.0f;

			float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
			float v =  (float)(((int)levelData[y][x]) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
			float spriteWidth = (1.0f / (float)SPRITE_COUNT_X)  - spacingX;
			float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y - spacingY;


			if ((int)levelData[y][x] == 0)
			{
				spriteWidth = 0.0f;
				spriteHeight = 0.0f;
			}
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
	float an = 16.0f;

		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &an);
	

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);
	//glBindTexture(GL_TEXTURE_2D, textureID);

	modelMatrix.identity();
	program->setModelMatrix(modelMatrix);

	glBindTexture(GL_TEXTURE_2D, tileset);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, an);
	glDrawArrays(GL_TRIANGLES, 0, 6*mapHeight*mapWidth);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY) {
	*gridX = (int)(worldX / TILE_SIZE);
	*gridY = (int)(-worldY / TILE_SIZE);
}
void resetLevelDefaults()
{
	Player.isAlive = true;
	Player.x = 0.5f;
	Player.y = -0.380f - 10.0f;
	Player.acceleration_x = 0.0f;
	Player.acceleration_y = 0.0f;
	Player.velocity_x = 0.0f;
	Player.velocity_y = 0.0f;
	for (Entity* e : enemiesOne)
	{
		delete e;
	}
	enemiesOne.clear();
	for (Entity* e : enemiesTwo)
	{
		delete e;
	}
	enemiesTwo.clear();
	for (Entity* e : enemiesThree)
	{
		delete e;
	}
	enemiesThree.clear();

}
void setup(){
	//SDL
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("William He - CS3113 Final Project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 852, 480, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 852, 480);
	projectionMatrix.setOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
	program = new ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	GLuint oreoTexture = loadTexture(RESOURCE_FOLDER"cookieBrown.png");
	GLuint spriteSheetTexture = loadTexture(RESOURCE_FOLDER"aliens.png");
	GLuint greenAlienSpriteSheet = loadTexture(RESOURCE_FOLDER"p1_spritesheet.png");
	GLuint enemyWorm = loadTexture(RESOURCE_FOLDER"slimeWalk1.png");
	enemyWormTexture = loadTexture(RESOURCE_FOLDER"slimeWalk1.png");
	backgroudTexture = loadTexture(RESOURCE_FOLDER"bg_grasslands.png");
	fontSheet = loadTexture(RESOURCE_FOLDER"font1.png");
	
	Mix_Init(MIX_INIT_MP3);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

	music = Mix_LoadMUS("song.mp3");
	Mix_PlayMusic(music, -1);
	jump = Mix_LoadWAV("jump_11.wav");

	glUseProgram(program->programID);
	
	for (int i = 0; i < 10; i++)
	{
		tiles[i] = new Entity(SheetSprite(oreoTexture, 0.0f, 0.0f, 1.0f, 1.0F, 0.6f), true, -2.7f + 1.5f*i, -0.8f);
	}	
	//Player =  Entity(SheetSprite(spriteSheetTexture, 676.0f / 1024.0f, 184.0f / 512.0f, 66.0f / 1024.0f, 92.0f /
	//	512.0f, 0.6f),false, 0.0f, -0.380f - 12.0f);
	Player = Entity(SheetSprite(greenAlienSpriteSheet, 67.0f / 508.0f, 196.0f / 288.0f, 66.0f / 508.0f, 92.0f /
		288.0f, 0.7f), false, 0.5f, -0.380f - 10.0f);
	Player.entityType = ENTITY_PLAYER;



	background = SheetSprite(backgroudTexture, 0.0f, 0.0f, 1024.0f, 512.0f, 7.0f);
	resetLevelDefaults();
	loadLevel();
	
}
void processEvents(){
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
		else if (event.type == SDL_KEYDOWN && (currentState == STATE_GAME || currentState == STATE_LEVEL2 || currentState == STATE_LEVEL3)){
			if (event.key.keysym.scancode == SDL_SCANCODE_SPACE && Player.collidedBottom){
				Mix_PlayChannel(-1, jump, 0);
				Player.velocity_y = 5.0f;
				Player.actionState = ACTION_JUMPING;

			}
		}
		else if (event.type == SDL_KEYUP && (currentState == STATE_GAME || currentState == STATE_LEVEL2 || currentState == STATE_LEVEL3))
		{
			if (Player.velocity_y > 0.5f)
			{
				Player.velocity_y = 2.0f;
			}
		}
	}
}

void resetLevelDefaults2()
{
	Player.isAlive = true;
	Player.x = 0.5f;
	Player.y = 0.0f;
	Player.acceleration_x = 0.0f;
	Player.acceleration_y = 0.0f;
	Player.velocity_x = 0.0f;
	Player.velocity_y = 0.0f;
	for (Entity* e : enemiesOne)
	{
		delete e;
	}
	enemiesOne.clear();
	for (Entity* e : enemiesTwo)
	{
		delete e;
	}
	enemiesTwo.clear();
	for (Entity* e : enemiesThree)
	{
		delete e;
	}
	enemiesThree.clear();
	
}
void resetLevelDefaults3()
{
	Player.isAlive = true;
	Player.x = 0.5f;
	Player.y = 0.0f;
	Player.acceleration_x = 0.0f;
	Player.acceleration_y = 0.0f;
	Player.velocity_x = 0.0f;
	Player.velocity_y = 0.0f;
	for (Entity* e : enemiesOne)
	{
		delete e;
	}
	enemiesOne.clear();
	for (Entity* e : enemiesTwo)
	{
		delete e;
	}
	enemiesTwo.clear();
	for (Entity* e : enemiesThree)
	{
		delete e;
	}
	enemiesThree.clear();
	
}
void penetrationUpdate(Entity* entity)
{
	entity->collidedBottom = false;
	entity->collidedTop = false;
	entity->collidedLeft = false;
	entity->collidedRight = false;
	int currX1, currY1, currX2, currY2;
	float bot = entity->y - entity->getHeight() * 0.5f;
	float top = entity->y + entity->getHeight() * 0.5f;
	float left = entity->x - entity->getWidth() * 0.5f;
	float right = entity->x + entity->getWidth() * 0.5f;
	float penetration = 0.0f;
//Penetration Bot
	worldToTileCoordinates(entity->x - (0.95f* entity->getWidth() / 2), entity->y - entity->getHeight() / 2, &currX1, &currY1);
	worldToTileCoordinates(entity->x + (0.95f* entity->getWidth() / 2), entity->y - entity->getHeight() / 2, &currX2, &currY2);
	bool penetration1 = std::find(std::begin(staticTilesIndex), std::end(staticTilesIndex), levelData[currY1][currX1]) != std::end(staticTilesIndex);
	bool penetration2 = std::find(std::begin(staticTilesIndex), std::end(staticTilesIndex), levelData[currY2][currX2]) != std::end(staticTilesIndex);
	bool lava1 = std::find(std::begin(lavaTilesIndex), std::end(lavaTilesIndex), levelData[currY1][currX1]) != std::end(lavaTilesIndex);
	bool lava2 = std::find(std::begin(lavaTilesIndex), std::end(lavaTilesIndex), levelData[currY2][currX2]) != std::end(lavaTilesIndex);
	if ((lava1 || lava2) && entity->entityType == ENTITY_PLAYER)
	{
		currentState = STATE_GAME_OVER;
	}
	if (penetration1 || penetration2)
	{
		penetration = fabs((-TILE_SIZE*(currY1)) - bot);
		entity->y += penetration + 0.000039f;
		entity->collidedBottom = true;
		entity->velocity_y = 0.0f;
	}

	//Penetrate Top
	worldToTileCoordinates(entity->x - (0.95f*entity->getWidth() / 2), entity->y + entity->getHeight() / 2, &currX1, &currY1);
	worldToTileCoordinates(entity->x + (0.95f*entity->getWidth() / 2), entity->y + entity->getHeight() / 2, &currX2, &currY2);
	penetration1 = std::find(std::begin(staticTilesIndex), std::end(staticTilesIndex), levelData[currY1][currX1]) != std::end(staticTilesIndex);
	penetration2 = std::find(std::begin(staticTilesIndex), std::end(staticTilesIndex), levelData[currY2][currX2]) != std::end(staticTilesIndex);
	if (penetration1 || penetration2)
	{
		penetration = fabs((-TILE_SIZE*(currY1)-TILE_SIZE) - top);
		entity->y -= penetration + 0.000009f;
		entity->collidedTop = true;
		entity->velocity_y = 0.0f;
	}

	//Penetrate Left
	worldToTileCoordinates(entity->x - entity->getWidth() / 2, entity->y, &currX1, &currY1);
	worldToTileCoordinates(entity->x - entity->getWidth() / 2, entity->y, &currX2, &currY2);
	penetration1 = std::find(std::begin(staticTilesIndex), std::end(staticTilesIndex), levelData[currY1][currX1]) != std::end(staticTilesIndex);
	penetration2 = std::find(std::begin(staticTilesIndex), std::end(staticTilesIndex), levelData[currY2][currX2]) != std::end(staticTilesIndex);
	if (penetration1 || penetration2)
	{
		penetration = fabs((TILE_SIZE * currX1 + TILE_SIZE) - left);
		entity->x += penetration + 0.00039f;
		entity->collidedLeft = true;
		entity->velocity_x = 5.0f;
	}

	//Penetrate Right
	worldToTileCoordinates(entity->x + entity->getWidth() / 2, entity->y + (0.95f*entity->getHeight()/2.0f), &currX1, &currY1);
	worldToTileCoordinates(entity->x + entity->getWidth() / 2, entity->y - (0.95f*entity->getHeight()/2.0f), &currX2, &currY2);
	penetration1 = std::find(std::begin(staticTilesIndex), std::end(staticTilesIndex), levelData[currY1][currX1]) != std::end(staticTilesIndex);
	penetration2 = std::find(std::begin(staticTilesIndex), std::end(staticTilesIndex), levelData[currY2][currX2]) != std::end(staticTilesIndex);
	if (penetration1 || penetration2)
	{
		penetration = fabs((TILE_SIZE * currX1) - right);
		entity->x -= penetration + 0.000009f;
		entity->collidedRight = true;
		entity->velocity_x = 0.0f;
	}

}
void edgeTurns(Entity* entity)
{
	
	//Create detector point
	// Slightly lower and to the right/left
	/*entity->collidedBottom = false;
	entity->collidedTop = false;
	entity->collidedLeft = false;
	entity->collidedRight = false;*/
	int currX1, currY1, currX2, currY2;
	float bot = entity->y - entity->getHeight() * 0.75f;
	float top = entity->y + entity->getHeight() * 0.5f;
	float left = entity->x - entity->getWidth() * 0.7f;
	float right = entity->x + entity->getWidth() * 0.65f;
	float penetration = 0.0f;
	worldToTileCoordinates(left, bot, &currX1, &currY1);
	worldToTileCoordinates(right, bot, &currX2, &currY2);
	bool penetration1 = std::find(std::begin(staticTilesIndex), std::end(staticTilesIndex), levelData[currY1][currX1]) != std::end(staticTilesIndex);
	bool penetration2 = std::find(std::begin(staticTilesIndex), std::end(staticTilesIndex), levelData[currY2][currX2]) != std::end(staticTilesIndex);

		if (!penetration2 && entity->collidedBottom)
		{
			if (entity->acceleration_x > 0.0f)
				entity->acceleration_x *= -1.0f;
		}
	
		else if (!penetration1 && entity->collidedBottom)
		{
			if (entity->acceleration_x <= 0.0f)
				entity->acceleration_x *= -1.0f;
		}


		if (entity->collidedLeft || entity->collidedRight)
		{
			entity->acceleration_x *= -1.0f;
		}
}
void detectEntity(Entity* entity, Entity* target)
{
	float bot = entity->y - entity->getHeight() * 0.5f;
	float top = entity->y + entity->getHeight() * 0.5f;
	float left = entity->x - entity->getWidth() * 0.5f;
	float right = entity->x + entity->getWidth() * 0.5f;
	float midHeight = entity->y;

	float targetBot = target->y - target->getHeight() * 0.5f;
	float targetTop = target->y + target->getHeight() * 0.5f;
	float targetLeft = target->x - target->getWidth() * 0.5f;
	float targetRight = target->x + target->getWidth() * 0.5f;

	float detectionRange = 2 * TILE_SIZE;
	float leftDetect = left - detectionRange;
	float rightDetect = right + detectionRange;
	
	if (leftDetect < targetLeft && targetRight < left && entity->enemyState != ENEMY_ANGRY && midHeight<targetTop && midHeight>targetBot){
			entity->enemyState = ENEMY_ANGRY;
			entity->acceleration_x = -2.0f;
		}
	
	
	else if (rightDetect > targetLeft && rightDetect < targetRight && midHeight<targetTop && midHeight>targetBot &&entity->enemyState != ENEMY_ANGRY){
			entity->enemyState = ENEMY_ANGRY;
			entity->acceleration_x = 2.0f;
		}
	
}
void update(float elapsed){
	if (currentState == STATE_GAME_WIN)
	{
		if (keys[SDL_SCANCODE_RETURN]){
			currentState = STATE_TITLE_SCREEN;
		}
	}
	if (currentState == STATE_TITLE_SCREEN)
	{
		if (keys[SDL_SCANCODE_RETURN]){
			
			currentState = latestLevel;

			
				if (latestLevel == STATE_GAME){
					if (firstTime){
						loadLevel();
						firstTime = false;
					}
					if (!Player.isAlive){
						resetLevelDefaults();
						loadLevel();
					}
				}
				else if (latestLevel == STATE_LEVEL2)
				{
					//loadLevel2();
					if (!Player.isAlive){
						resetLevelDefaults2();
						loadLevel2();
					}
				}
				else if (latestLevel == STATE_LEVEL3)
				{
					//loadLevel2();
					if (!Player.isAlive){
						resetLevelDefaults3();
						loadLevel3();
					}
				}
			
		}
		if (keys[SDL_SCANCODE_Q]){
			done = true;
		}
		if (keys[SDL_SCANCODE_R]){
			currentState = latestLevel;
			if (latestLevel == STATE_GAME){
				//loadLevel();
				resetLevelDefaults();
				loadLevel();
			}
			else if (latestLevel == STATE_LEVEL2)
			{
				//loadLevel2();
				resetLevelDefaults2();
				loadLevel2();
			}
			else if (latestLevel == STATE_LEVEL3)
			{
				//loadLevel2();
				resetLevelDefaults3();
				loadLevel3();
			}
		}
		if (keys[SDL_SCANCODE_1]){
			currentState = STATE_GAME;
			latestLevel = STATE_GAME;
			resetLevelDefaults();
			loadLevel();
			/*if (!Player.isAlive){
				resetLevelDefaults();
			}*/
		}
		else if (keys[SDL_SCANCODE_2]){
			currentState = STATE_LEVEL2;
			latestLevel = STATE_LEVEL2;
			resetLevelDefaults2();
			loadLevel2();
			if (!Player.isAlive){
				//resetLevelDefaults();
			}
		}
		else if (keys[SDL_SCANCODE_3]){
			currentState = STATE_LEVEL3;
			latestLevel = STATE_LEVEL3;
			resetLevelDefaults3();
			loadLevel3();
			if (!Player.isAlive){
				//resetLevelDefaults();
			}
		}
	}
	else if (currentState == STATE_GAME_OVER)
	{
		if (keys[SDL_SCANCODE_RETURN])
		{
			currentState = latestLevel;
			if (latestLevel == STATE_GAME){
				//loadLevel();
				resetLevelDefaults();
				loadLevel();
			}
			else if (latestLevel == STATE_LEVEL2)
			{
				//loadLevel2();
				resetLevelDefaults2();
				loadLevel2();
			}
			else if (latestLevel == STATE_LEVEL3)
			{
				//loadLevel2();
				resetLevelDefaults3();
				loadLevel3();
			}
		}
	}
	else if (currentState == STATE_GAME)
	{
		if (keys[SDL_SCANCODE_F10])
		{
			currentState = STATE_TITLE_SCREEN;
		}
		if (keys[SDL_SCANCODE_LEFT] && !Player.collidedLeft && !Player.actionState == ACTION_JUMPING){
			Player.acceleration_x = -4.0f;
		}
		else if (keys[SDL_SCANCODE_LEFT] && !Player.collidedLeft && Player.actionState == ACTION_JUMPING){
			Player.acceleration_x = -2.0f;
		}
		else if (keys[SDL_SCANCODE_RIGHT] && !Player.collidedRight && !Player.actionState == ACTION_JUMPING){
			Player.acceleration_x = 4.0f;
		}
		else if (keys[SDL_SCANCODE_RIGHT] && !Player.collidedRight && Player.actionState == ACTION_JUMPING){
			Player.acceleration_x = 2.0f;
		}
		else{
			Player.acceleration_x = 0.0f;
		}

		

		animationElapsed += elapsed;
		if (animationElapsed > 1.0 / framesPerSecond) {
			currentIndex++;
			animationElapsed = 0.0;
			if (currentIndex > numFrames - 1) {
				currentIndex = 0;
			}
		}

		penetrationUpdate(&Player);
//level 1
		if (currentState == STATE_GAME){
			Player.update(elapsed, FRICTION, GRAVITY);

			for (size_t i = 0; i < enemiesOne.size(); i++)
			{
				penetrationUpdate(enemiesOne[i]);
				Player.performCollision(enemiesOne[i]);
				enemiesOne[i]->update(elapsed, FRICTION, GRAVITY);
				
					edgeTurns(enemiesOne[i]);
				
					detectEntity(enemiesOne[i], &Player);
				
			}
		}
	
//level 2=================================================================================================================================================
		if (currentState == STATE_LEVEL2){
			Player.update(elapsed, FRICTION/2.0f, GRAVITY);
			for (size_t i = 0; i < enemiesTwo.size(); i++)
			{
				penetrationUpdate(enemiesTwo[i]);
				Player.performCollision(enemiesTwo[i]);
				enemiesTwo[i]->update(elapsed, FRICTION/2.0f, GRAVITY);
				edgeTurns(enemiesTwo[i]);

			}
		}
		if (Player.isAlive && (Player.x >= portalx - TILE_SIZE) && Player.x <= portalx + TILE_SIZE){
			currentState = STATE_LEVEL2;
			latestLevel = STATE_LEVEL2;
			resetLevelDefaults2();
			loadLevel2();
		}
		if (!Player.isAlive && (currentState == STATE_GAME || currentState == STATE_LEVEL2)){
			currentState = STATE_GAME_OVER;
		}
	}
//Level 2 ==================================================================================================================
	else if (currentState == STATE_LEVEL2)
	{
		if (keys[SDL_SCANCODE_F10])
		{
			currentState = STATE_TITLE_SCREEN;
		}
		if (keys[SDL_SCANCODE_LEFT] && !Player.collidedLeft && !Player.actionState == ACTION_JUMPING){
			Player.acceleration_x = -4.0f;
		}
		else if (keys[SDL_SCANCODE_LEFT] && !Player.collidedLeft && Player.actionState == ACTION_JUMPING){
			Player.acceleration_x = -2.0f;
		}
		else if (keys[SDL_SCANCODE_RIGHT] && !Player.collidedRight && !Player.actionState == ACTION_JUMPING){
			Player.acceleration_x = 4.0f;
		}
		else if (keys[SDL_SCANCODE_RIGHT] && !Player.collidedRight && Player.actionState == ACTION_JUMPING){
			Player.acceleration_x = 2.0f;
		}
		else{
			Player.acceleration_x = 0.0f;
		}

		animationElapsed += elapsed;
		if (animationElapsed > 1.0 / framesPerSecond) {
			currentIndex++;
			animationElapsed = 0.0;
			if (currentIndex > numFrames - 1) {
				currentIndex = 0;
			}
		}

		penetrationUpdate(&Player);
			Player.update(elapsed, FRICTION / 2.0f, GRAVITY);
			for (size_t i = 0; i < enemiesTwo.size(); i++)
			{
				penetrationUpdate(enemiesTwo[i]);
				Player.performCollision(enemiesTwo[i]);
				enemiesTwo[i]->update(elapsed, FRICTION , GRAVITY);
				edgeTurns(enemiesTwo[i]);

			}
		
			if (Player.isAlive && (Player.x >= portalx - TILE_SIZE) && Player.x <= portalx + TILE_SIZE){
				currentState = STATE_LEVEL3;
				latestLevel = STATE_LEVEL3;
				resetLevelDefaults3();
				loadLevel3();
			}
		if (!Player.isAlive && (currentState == STATE_GAME || currentState == STATE_LEVEL2)){
			currentState = STATE_GAME_OVER;
		}
	}
//Level 3======================================================================================================================================
	else if (currentState == STATE_LEVEL3)
	{
		if (keys[SDL_SCANCODE_F10])
		{
			currentState = STATE_TITLE_SCREEN;
		}
		if (keys[SDL_SCANCODE_LEFT] && !Player.collidedLeft && !Player.actionState == ACTION_JUMPING){
			Player.acceleration_x = -4.0f;
		}
		else if (keys[SDL_SCANCODE_LEFT] && !Player.collidedLeft && Player.actionState == ACTION_JUMPING){
			Player.acceleration_x = -2.0f;
		}
		else if (keys[SDL_SCANCODE_RIGHT] && !Player.collidedRight && !Player.actionState == ACTION_JUMPING){
			Player.acceleration_x = 4.0f;
		}
		else if (keys[SDL_SCANCODE_RIGHT] && !Player.collidedRight && Player.actionState == ACTION_JUMPING){
			Player.acceleration_x = 2.0f;
		}
		else{
			Player.acceleration_x = 0.0f;
		}

		animationElapsed += elapsed;
		if (animationElapsed > 1.0 / framesPerSecond) {
			currentIndex++;
			animationElapsed = 0.0;
			if (currentIndex > numFrames - 1) {
				currentIndex = 0;
			}
		}

		penetrationUpdate(&Player);
			Player.update(elapsed, FRICTION * 2.0f, GRAVITY);
			for (size_t i = 0; i < enemiesThree.size(); i++)
			{
				penetrationUpdate(enemiesThree[i]);
				Player.performCollision(enemiesThree[i]);
				enemiesThree[i]->update(elapsed, FRICTION, GRAVITY);
				edgeTurns(enemiesThree[i]);
				detectEntity(enemiesThree[i], &Player);
			}
		
			if (Player.isAlive && (Player.x >= portalx - TILE_SIZE) && Player.x <= portalx + TILE_SIZE){
				currentState = STATE_GAME_WIN;
			}
		if (!Player.isAlive && (currentState == STATE_GAME || currentState == STATE_LEVEL2 || currentState == STATE_LEVEL3)){
			currentState = STATE_GAME_OVER;
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
	glClearColor(22 / 255.0f, 160 / 255.0f, 133 / 255.0f, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	if (currentState == STATE_TITLE_SCREEN)
	{
		modelMatrix.identity();
		modelMatrix.Translate(-1.4f, 1.5f, 0.0f);
		viewMatrix.identity();
		program->setModelMatrix(modelMatrix);
		DrawText(program, fontSheet, "Platformer Demo", 0.3f, -0.1f);

		modelMatrix.identity();
		modelMatrix.Translate(-1.37f, 0.0f, 0.0f);
		program->setModelMatrix(modelMatrix);
		DrawText(program, fontSheet, "Press enter here to start/continue", 0.2f, -0.1f);
		modelMatrix.identity();
		modelMatrix.Translate(-1.3f, 0.5f, 0.0f);
		program->setModelMatrix(modelMatrix);
		DrawText(program, fontSheet, "Press Q here to quit game", 0.2f, -0.1f);
		modelMatrix.identity();
		modelMatrix.Translate(-1.3f, 1.0f, 0.0f);
		program->setModelMatrix(modelMatrix);
		DrawText(program, fontSheet, "Press 1, 2, or 3 to select levels", 0.2f, -0.1f);
		modelMatrix.identity();
		modelMatrix.Translate(-1.3f, -0.5f, 0.0f);
		program->setModelMatrix(modelMatrix);
		DrawText(program, fontSheet, "Press R here to restart a game/ F10 for menu", 0.2f, -0.1f);

		modelMatrix.identity();
		modelMatrix.Translate(-0.50f, -1.0f, 0.0f);
		program->setModelMatrix(modelMatrix);
		DrawText(program, fontSheet, "Space =  jump ", 0.17f, -0.07f);
		modelMatrix.identity();
		modelMatrix.Translate(-0.57f, -1.5f, 0.0f);
		program->setModelMatrix(modelMatrix);
		DrawText(program, fontSheet, "Left/Right = move ", 0.17f, -0.07f);
	}

	if (currentState == STATE_GAME){

		/*
		
		modelMatrix.identity();
		program->setModelMatrix(modelMatrix);
		float bgTexCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glBindTexture(GL_TEXTURE_2D, backgroudTexture);
		float bgVertices[] = { -3.55f, -2.0f, 3.55f, -2.0, 3.55f, 2.0f, -3.55f, -2.0f, 3.55f, 2.0f, -3.55f, 2.0f };
		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, bgVertices);
		glEnableVertexAttribArray(program->positionAttribute);
		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, bgTexCoords);
		glEnableVertexAttribArray(program->texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program->positionAttribute);
		*/
		
		viewMatrix.identity();
		
			//viewMatrix.Scale(0.9f, 0.9f, 1.0f);
		viewMatrix.Translate(-1 * Player.x, -1 * Player.y, 0.0f);
		program->setViewMatrix(viewMatrix);
		renderLevel();
		if (Player.acceleration_x == 0.0f){
			/*Player.sprite.u = 67.0f / 508.0f;
			Player.sprite.v = 196.0f / 288.0f;
			Player.sprite.width = 66.0f / 508.0f;
			Player.sprite.height = 92.0f / 288.0f;*/
			
			
			/*if ((Player.acceleration_y) && !Player.collidedBottom)
			{
				Player.sprite.u = 438.0f / 508.0f;
				Player.sprite.v = 93.0f / 288.0f;
				Player.sprite.width = 67.0f / 508.0f;
				Player.sprite.height = 94.0f / 288.0f;
				Player.sprite.Draw(program, modelMatrix, Player.x, Player.y);
			}
			else {*/
				Player.sprite.u = 67.0f / 508.0f;
				Player.sprite.v = 196.0f / 288.0f;
				Player.sprite.width = 66.0f / 508.0f;
				Player.sprite.height = 92.0f / 288.0f;
				Player.sprite.Draw(program, modelMatrix, Player.x, Player.y);
			//}
		}
		else{
			Player.sprite.DrawUniformSheet(program, modelMatrix, Player.x, Player.y, runAnimation[currentIndex], 7, 3);
		}
		//Player.sprite.Draw(program, modelMatrix, Player.x, Player.y);
		//if (Enemy.isAlive)
			//Enemy.sprite.Draw(program, modelMatrix, Enemy.x, Enemy.y);
		//Enemy2.sprite.Draw(program, modelMatrix, Enemy2.x, Enemy2.y);
		std::cout << placeXD << Player.isAlive << Enemy.isAlive;
		//background.Draw(program, modelMatrix, 0.0f, 0.0f);
		for (size_t i = 0; i < enemiesOne.size(); i++)
		{
			enemiesOne[i]->sprite.Draw(program, modelMatrix, enemiesOne[i]->x, enemiesOne[i]->y);
		}
		std::cout << Player.x << endl;
		
	}
	if (currentState == STATE_LEVEL2){

		renderLevel();
		if (Player.acceleration_x == 0.0f){

			Player.sprite.u = 67.0f / 508.0f;
			Player.sprite.v = 196.0f / 288.0f;
			Player.sprite.width = 66.0f / 508.0f;
			Player.sprite.height = 92.0f / 288.0f;
			Player.sprite.Draw(program, modelMatrix, Player.x, Player.y);
			
		}
		else{
			Player.sprite.DrawUniformSheet(program, modelMatrix, Player.x, Player.y, runAnimation[currentIndex], 7, 3);
		}

		for (size_t i = 0; i < enemiesTwo.size(); i++)
		{
			enemiesTwo[i]->sprite.Draw(program, modelMatrix, enemiesTwo[i]->x, enemiesTwo[i]->y);
		}
		viewMatrix.identity();
		viewMatrix.Translate(-1 * Player.x, -1 * Player.y, 0.0f);
		program->setViewMatrix(viewMatrix);
	}
	if (currentState == STATE_LEVEL3){

		renderLevel();
		if (Player.acceleration_x == 0.0f){

			Player.sprite.u = 67.0f / 508.0f;
			Player.sprite.v = 196.0f / 288.0f;
			Player.sprite.width = 66.0f / 508.0f;
			Player.sprite.height = 92.0f / 288.0f;
			Player.sprite.Draw(program, modelMatrix, Player.x, Player.y);

		}
		else{
			Player.sprite.DrawUniformSheet(program, modelMatrix, Player.x, Player.y, runAnimation[currentIndex], 7, 3);
		}

		for (size_t i = 0; i < enemiesThree.size(); i++)
		{
			enemiesThree[i]->sprite.Draw(program, modelMatrix, enemiesThree[i]->x, enemiesThree[i]->y);
		}
		viewMatrix.identity();
		viewMatrix.Translate(-1 * Player.x, -1 * Player.y, 0.0f);
		program->setViewMatrix(viewMatrix);
	}
	if (currentState == STATE_GAME_OVER){
		modelMatrix.identity();
		modelMatrix.Translate(-1.2f, 1.5f, 0.0f);
		viewMatrix.identity();
		program->setModelMatrix(modelMatrix);
		DrawText(program, fontSheet, "Game over", 0.3f, -0.1f);
		modelMatrix.identity();
		modelMatrix.Translate(-1.67f, 0.0f, 0.0f);
		program->setModelMatrix(modelMatrix);
		DrawText(program, fontSheet, "Press enter to continue", 0.2f, 0.0f);
		modelMatrix.identity();
		modelMatrix.Translate(-0.50f, -0.5f, 0.0f);
		program->setModelMatrix(modelMatrix);
		DrawText(program, fontSheet, "Space =  jump ", 0.17f, -0.07f);
		modelMatrix.identity();
		modelMatrix.Translate(-0.57f, -0.7f, 0.0f);
		program->setModelMatrix(modelMatrix);
		DrawText(program, fontSheet, "Left/Right = move ", 0.17f, -0.07f);
	}
	if (currentState == STATE_GAME_WIN){
		modelMatrix.identity();
		modelMatrix.Translate(-1.2f, 1.5f, 0.0f);
		viewMatrix.identity();
		program->setModelMatrix(modelMatrix);
		DrawText(program, fontSheet, "You win", 0.3f, -0.1f);
	}
	SDL_GL_SwapWindow(displayWindow);
	
}

void cleanUp(){
	Mix_FreeChunk(jump);
	Mix_FreeMusic(music);
	Mix_CloseAudio();
	Mix_Quit();
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
