#pragma once

#include "SFML/Graphics.hpp"
#include "ParticleSys.h"
#include "Utils.h"
#include "GameObj.h"
#include "MyDB.h"

/*
A box to put Games Constants in.
These are special numbers with important meanings (screen width,
ascii code for the escape key, number of lives a player starts with,
the name of the title screen music track, etc.
*/
namespace GC
{
	//game play related constants to tweak
	const Dim2Di SCREEN_RES{ 1200,800 };
	const float SPEED = 250.f;			//ship speed
	const float SCREEN_EDGE = 0.6f;		//how close to the edge the ship can get
	const char ESCAPE_KEY{ 27 };
	const char BACKSPACE_KEY{ 8 };
	const float ROCK_MIN_DIST = 2.15f;	//used when placing rocks to stop them getting too close
	const int NUM_ROCKS = 50;			//how many to place
	const int PLACE_TRIES = 10;			//how many times to try and place before giving up
	const float ROCK_SPEED = 150.f;
	const Dim2Df ROCK_RAD{ 10.f,40.f };
	const int NUM_BULLETS = 50;
	const int NUM_ENEMIES = 50;
	const float ENEMY_SPEED = 150;
	const float ENEMY_BULLET_SPEED = 400;
	const int NUM_LIVES = 3;
}

/*
Data about the game for high scores and possibly
tracking stats about players
*/
struct Metrics {
	const std::string VERSION = "1.4";	//change this every time the game changes significantly
	int score;				//current session score
	int lives;				//current session lives
	std::string name;		//current player
	bool useDB = true;		//database or text file?
	MyDB db;	//a database wrapper object for sqlite

	//package some data into an object so we can store them in a container
	struct PlayerData {
		std::string name;
		int score;
	};
	std::vector<PlayerData> playerData;		//info about the last 10 players
	std::string filePath;		//where we are storing the data

	//reset for a new game e.g. score=zero, lives=3, etc.
	void Restart();
	//does the current session score qualify for the top 10?
	bool IsScoreInTopTen();
	/*
	Decide if the current sessoin score qualifies for the top ten
	If it does then add it and resort the playerData vector
	playerData[0].score should be the highest and playerData[9].score the lowest
	*/
	void SortAndUpdatePlayerData();
	/*
	Load player data, this could be a text file or a database
	*/
	bool Load(const std::string& path, bool _useDB) {
		useDB = _useDB;
		return (useDB) ? DBLoad(path) : FileLoad(path);
	}
	/*
	Save all playerData, the path is optional as playerData
	must have previously been loaded so we should have a path
	*/
	bool Save(const std::string& path = "") {
		return (useDB) ? DBSave(path) : FileSave(path);
	}

	//stream to and from a text file
	bool FileSave(const std::string& path = "");
	bool FileLoad(const std::string& path);
	//load and save from a database - needs writing
	bool DBSave(const std::string& path = "");
	bool DBLoad(const std::string& path);
};


/*
Manage the asteroid dodging game
*/
struct Game
{
	//textures we are going to need, load them once only
	
	sf::Texture texChar;
	sf::Texture texbckgrnd;
	sf::Texture texRock;
	sf::Texture texBullet;
	sf::Texture texEnemy;

	std::vector<GameObj> objects;	//anything moving around
	
	SpawnTimer rockTimer;	//we need timers so rocks and enemies appear slowly
	SpawnTimer enemyTimer;
	float rockShipClearance;	//when placing an asteroid, how many ship lengths away from other rocks should it be, harder = smaller
	ParticleSys particleSys;	//this object makes pretty explosions
	sf::Font font;		//we need a font to use
	Metrics metrics;	//an object to record info about the player, statistics
	float timer = 0;	//like a main clock for the whole game, useful when timing things
		
	//load textures, create ship and rocks, set all rocks initially inactive
	void Init(sf::RenderWindow& window);
	/*move the ship and rocks, spawn new rocks 
	window - need it to draw things, get the size of it, etc.
	elapsed - frame time for any physics code
	*/
	void Update(sf::RenderWindow& window, float elapsed, bool fire, char key);
	//draw everything, called once a frame. Still includes elapsed time incase anything is rotating/scaling
	void Render(sf::RenderWindow& window, float elapsed);
	//randomly put rocks on the screen at the start, quite tricky as they need carefully spacing out
	//window - so you know how big the space is
	//tex - rock texture
	void PlaceRocks(sf::RenderWindow& window, sf::Texture& tex);
	//similar to the above, but used once the game is running to reuse old rocks
	void PlaceExistingRocks(sf::RenderWindow& window);
	

	//we need to control what is going on in the game, start->play->die->enterName
	enum class Mode { INTRO, GAME, GAME_OVER, ENTER_NAME};
	Mode mode = Mode::INTRO;

	//things to render over the game, like scores
	void RenderHUD(sf::RenderWindow& window, float elapsed, sf::Font & font);
	//called every time a new game starts to reset everything
	void NewGame(sf::RenderWindow & window);

	//it's an update function, but only call it when the game is running
	//as it's going to be the most complex update compared to intro mode and when the game is over
	void UpdateInGame(sf::RenderWindow & window, float elapsed, bool fire);
	//separate render function just for the game over screen
	void RenderGameOver(sf::RenderWindow & window, float elapsed);
};

/*
Update every object to see if it is colliding with any other - sets the colliding flag true
objects - any could be colliding
debug - if true, draw the collision radius and mark any collisions in red
*/
void CheckCollisions(std::vector<GameObj>& objects, sf::RenderWindow& window, bool debug = true);
//
void DrawCircle(sf::RenderWindow& window, const sf::Vector2f& pos, float radius, sf::Color col);
/*
file - path and file name and extension
tex - set this up with the texture
*/
bool LoadTexture(const std::string& file, sf::Texture& tex);
/*
Check if two circles are touching
pos1,pos2 - two centres
minDist - minimum colliding distance
*/
bool CircleToCircle(const sf::Vector2f& pos1, const sf::Vector2f& pos2, float minDist);
/*
Test one object against an array of other objects to see if it collides
It's OK if the object happens to be in the array, it won't test against itself
*/
bool IsColliding(GameObj& obj, std::vector<GameObj>& objects);
/*
Setup a new rock to fly in from the right
Look through the objects array, find an inactive rock, pick a new starting position
for it just off screen to the right. Check it is at least extraClearance units away
from anything else and mark active.
If it does collide with something then don't spawn and return false.
*/
bool Spawn(GameObj::ObjectT type, sf::RenderWindow& window, std::vector<GameObj>& objects, float extraClearance);
