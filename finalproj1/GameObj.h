#pragma once


#include "SFML/Graphics.hpp"
#include "Utils.h"

struct Game;

/*
A game object is anything represented by a sprite that exists in the game world
Rocks, the player's ship, enemy ships, bullets - all game objects with sprites, hela
*/
struct GameObj
{
	sf::Sprite spr;	//main image
	sf::Sprite bg;
	float radius = 0;				//collision radius
	enum class ObjectT { Ship, Rock, Bullet, Enemy,Background };	//what is this object instance?
	ObjectT type = ObjectT::Rock;	//what type am I?	
	bool colliding = false;			//did we hit something on the last update
	bool active = false;			//should we be updating and rendering this one?
	int health = 0;					//if it's zero then I'm dead
	Game *pGame = nullptr;			//keep a pointer (a handle) to my owner the game object
	GameObj *pMySpawner = nullptr;
	bool bga = false;	//if I am a bullet, then some other object fired me off (player or enemy?)

	/*
	Call this to setup your object
	window - sfml render window
	tex - texture to use on the sprite
	type - what is it meant to be
	game - a reference to our owner the game itself
	*/
	void Init(sf::RenderWindow& window, sf::Texture& tex, ObjectT type_, Game& game);
	/*called by Init as needed
	window - sfml render window
	tex - player ship texture
	*/
	void InitChar(sf::RenderWindow& window, sf::Texture& tex);
	/*Called when we want to start, reset various player ship data
	windows - sfml render window
	*/
	void ResetShip(sf::RenderWindow& window);
	//called by Init as needed, like InitShip but for rocks
	void InitRock(sf::RenderWindow& window, sf::Texture& tex);
	//like resetShip(), if we want a new rock, we call this to reset variables
	void ResetRock();
	//These two are like the others but for enemy ships
	void InitEnemy(sf::RenderWindow& window, sf::Texture& tex);
	void ResetEnemy();
	/*move and update logic
	window - usual render window
	elapsed - physics simulation needs frame time 1/60th a second or similar
	fire - did the player want to shoot
	*/
	void InitBackground(sf::RenderWindow& window, sf::Texture& tex);
	void Update(sf::RenderWindow& window, float elapsed, bool fire);
	//draw yourself
	//need the window to draw and elapsed time might be needed if there's any motion or spinning or scaling
	void Render(sf::RenderWindow& window, float elapsed);
	/*handle moving the ship around
	screenSz - width and height of the screen
	elapsed - frame time
	fire - let off a bullet
	*/
	void PlayerControl(const sf::Vector2u& screenSz, float elapsed, bool fire);
	//rocks all move left, when leave the left edge of the screen they deactivate
	//elapsed time is needed for smooth motion
	void MoveRock(float elapsed);

	//these three functions are similar, but specifically for bullets which 
	//simply fly to the right/left and go to sleep if they leave the screen
	//if they are spawn by a player go right, if an enemy then go left
	void MoveBullet(const sf::Vector2u& screenSz, float elapsed);
	void InitBullet(sf::RenderWindow& window, sf::Texture& tex);
	void FireBullet(const sf::Vector2f& pos);

	//similar again, enemies fly to the left
	void MoveEnemy(const sf::Vector2u& screenSz, float elapsed);
	//enemies fire bullets too, but need timers to slow firing down so it isn't too hard
	void EnemyShoot(float elapsed);

	/*what should an object do if it hits another object, for example, a bullet hitting an enemy
	other - the other object it hit
	*/
	void Hit(GameObj& other);
	/*After taking a hit we might decide we need to take some damage
	so if an asteroid takes so much damage it dies then it needs to explode and disappear
	amount - how much damage
	other - the other object doing damage to us
	*/
	void TakeDamage(int amount, GameObj& other);
};

