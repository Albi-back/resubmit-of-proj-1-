#include <assert.h>
#include <string>
#include <math.h>
#include <sstream>
#include <iomanip>
#include <fstream>

#include "Game.h"

using namespace sf;
using namespace std;

void Metrics::SortAndUpdatePlayerData() {
	PlayerData d{ name,score };
	playerData.push_back(d);
}


bool Metrics::DBLoad(const std::string& path) {
	
	return false;
}

bool Metrics::FileSave(const std::string& path) {

	if(!path.empty())
		filePath = path;
	ofstream fs;
	fs.open(filePath);
	if (fs.is_open() && fs.good())
	{
		fs << VERSION;
		for (size_t i = 0; i < playerData.size(); ++i)
		{
			fs << ' ' << playerData[i].name << ' ' << playerData[i].score;
		}
		assert(!fs.fail());
		fs.close();
	}
	else
	{
		assert(false);
		return false;
	}
	return true;
}

bool Metrics::DBSave(const std::string& path) {
	
	return false;
}

bool Metrics::FileLoad(const std::string& path) {

	assert(!path.empty());
	filePath = path;
	ifstream fs;
	fs.open(filePath, ios::binary);
	if (fs.is_open() && fs.good())
	{
		string version;
		fs >> version;
		if (version == VERSION)
		{
			playerData.clear();
			while (!fs.eof()) {
				PlayerData d;
				fs >> d.name;
				assert(!d.name.empty());
				fs >> d.score;
				assert(d.score >= 0);
				playerData.push_back(d);
			}
		}
		assert(!fs.fail());
		fs.close();
	}
	return false;
}

bool Metrics::IsScoreInTopTen() {
	if (playerData.size() < 10)
		return true;
	return playerData.back().score < score;
}

void Metrics::Restart() {
	score = 0;
	lives = GC::NUM_LIVES;
}


bool LoadTexture(const string& file, Texture& tex)
{
	if (tex.loadFromFile(file))
	{
		tex.setSmooth(true);
		tex.setRepeated(true);
		return true;
	}
	//else if (tex.loadFromFile("data/bckgd1.jpg"))
	//{
		//tex.setSmooth(true);
		//tex.setRepeated(true); 
		//return true;

	//}
	assert(false);
	return false;
}


void DrawCircle(RenderWindow& window, const Vector2f& pos, float radius, Color col)
{
	CircleShape c;
	c.setRadius(radius);
	c.setPointCount(20);
	c.setOutlineColor(col);
	c.setOutlineThickness(2);
	c.setFillColor(Color::Transparent);
	c.setPosition(pos);
	c.setOrigin(radius, radius);
	window.draw(c);
}

bool CircleToCircle(const Vector2f& pos1, const Vector2f& pos2, float minDist)
{
	float dist = (pos1.x - pos2.x) * (pos1.x - pos2.x) +
		(pos1.y - pos2.y) * (pos1.y - pos2.y);
	dist = sqrtf(dist);
	return dist <= minDist;
}

void CheckCollisions(vector<GameObj>& objects, RenderWindow& window, bool debug)
{
	if (objects.size() > 1)
	{
		for (size_t i = 0; i < objects.size(); ++i)
		{
			GameObj& a = objects[i];
			if (a.active)
			{
				if (i < (objects.size() - 1))
					for (size_t ii = i + 1; ii < (objects.size()); ++ii)
					{
						GameObj& b = objects[ii];
						if (b.active)
						{
							if (CircleToCircle(a.spr.getPosition(), b.spr.getPosition(), a.radius + b.radius))
							{
								a.colliding = true;
								b.colliding = true;
								a.Hit(b);
								b.Hit(a);
							}
						}
					}
				if (debug)
				{
					Color col = Color::Green;
					if (a.colliding)
						col = Color::Red;
					DrawCircle(window, a.spr.getPosition(), a.radius, col);
				}
			}
		}
	}
}


bool IsColliding(GameObj& obj, vector<GameObj>& objects)
{
	assert(obj.active);
	size_t idx = 0;
	bool colliding = false;
	while (idx < objects.size() && !colliding) {

		if (&obj != &objects[idx] && objects[idx].active)
		{
			const Vector2f& posA = obj.spr.getPosition();
			const Vector2f& posB = objects[idx].spr.getPosition();
			float dist = obj.radius + objects[idx].radius;
			colliding = CircleToCircle(posA, posB, dist);
		}
		++idx;
	}
	return colliding;
}

void Game::PlaceExistingRocks(RenderWindow& window)
{
	for (size_t i = 0; i < objects.size(); ++i)
	{
		if (objects[i].type == GameObj::ObjectT::Rock)
		{
			GameObj& rock = objects[i];
			rock.radius *= GC::ROCK_MIN_DIST;
			rock.active = true;
			int tries = 0;
			do {
				tries++;
				float x = (float)(rand() % window.getSize().x);
				float y = (float)(rand() % window.getSize().y);
				rock.spr.setPosition(x, y);
			} while (tries < GC::PLACE_TRIES && IsColliding(rock, objects));
			rock.radius *= 1 / GC::ROCK_MIN_DIST;
		}
	}
}

void Game::PlaceRocks(RenderWindow& window, Texture& tex)
{
	bool space = true;
	int ctr = GC::NUM_ROCKS;
	while (space && ctr)
	{
		GameObj rock;
		rock.Init(window, tex, GameObj::ObjectT::Rock,*this);
		rock.radius *= GC::ROCK_MIN_DIST;
		int tries = 0;
		do {
			tries++;
			float x = (float)(rand() % window.getSize().x);
			float y = (float)(rand() % window.getSize().y);
			rock.spr.setPosition(x, y);
		} while (tries < GC::PLACE_TRIES && IsColliding(rock, objects));
		rock.radius *= 1 / GC::ROCK_MIN_DIST;
		if (tries != GC::PLACE_TRIES)
			objects.push_back(rock);
		else
			space = false;
		--ctr;
	}
}

bool Spawn(GameObj::ObjectT type, RenderWindow& window, vector<GameObj>& objects, float extraClearance)
{
	size_t idx = 0;
	bool found = false;
	while (idx < objects.size() && !found)
	{
		GameObj& obj = objects[idx];
		if (!obj.active && obj.type==type)
			found = true;
		else
			++idx;
	}

	if (found)
	{
		GameObj& obj = objects[idx];
		switch (type)
		{
		case GameObj::ObjectT::Rock:
			obj.ResetRock();
			break;
		case GameObj::ObjectT::Enemy:
			obj.ResetEnemy();
			break;
		default:
			assert(false);
		}
		obj.active = true;
		obj.radius += extraClearance;
		FloatRect r = obj.spr.getGlobalBounds();
		float y = (r.height/2.f) + (rand() % (int)(window.getSize().y - r.height));
		obj.spr.setPosition(window.getSize().x + r.width, y);
		if (IsColliding(obj, objects))
		{
			found = false;
			obj.active = false;
		}
		obj.radius -= extraClearance;
	}
	return found;
}

void Game::Init(sf::RenderWindow & window) {
	
	LoadTexture("data/Knight.png",texChar);
	LoadTexture("data/bckgd1.jpg", texbckgrnd);
	
	
	if (!font.loadFromFile("data/fonts/comic.ttf"))
		assert(false);

	objects.clear();
	GameObj obj;
	objects.insert(objects.begin(), GC::NUM_ROCKS+GC::NUM_BULLETS+1+GC::NUM_ENEMIES, obj);
	
	size_t idx = 0, total=0;
	
	objects[idx++].Init(window, texChar, GameObj::ObjectT::Ship, *this);

	
	
	
	
	
	total += GC::NUM_ROCKS + 1;
	

	rockShipClearance = objects[0].spr.getGlobalBounds().width * 2.f;

	//PlaceExistingRocks(window);

	//particleSys.Init();

	metrics.Load("data/scores.txt", false);
}

void Game::NewGame(sf::RenderWindow & window)
{
	for (size_t i = 1; i < objects.size(); ++i)
		objects[i].active = false;
	objects[0].ResetShip(window);
	rockTimer.Reset(0.5f, 1);
	enemyTimer.Reset(2.f, 0.5f);
}

void Game::UpdateInGame(sf::RenderWindow & window, float elapsed, bool fire) {
	if (rockTimer.Cycle(elapsed))
	{
		if (Spawn(GameObj::ObjectT::Rock, window, objects, rockShipClearance))
			rockTimer.Reset();
	}

	if (enemyTimer.Cycle(elapsed))
	{
		if (Spawn(GameObj::ObjectT::Enemy, window, objects, objects[0].spr.getGlobalBounds().width * 2))
			enemyTimer.Reset();
	}

	CheckCollisions(objects, window, false);
	for (size_t i = 0; i < objects.size(); ++i)
		objects[i].Update(window, elapsed, fire);

//	particleSys.Update(elapsed);

	if (metrics.lives <= 0 && !particleSys.cache.IsBusy() && particleSys.GetNumActiveEmitters() == 0) {
		//game over
		if (metrics.IsScoreInTopTen() && metrics.name.empty())
			mode = Mode::ENTER_NAME;
		else
		{
			mode = Mode::GAME_OVER;
			metrics.SortAndUpdatePlayerData();
			metrics.Save();
		}
		timer = 0;
	}
}

void Game::Update(sf::RenderWindow & window, float elapsed, bool fire, char key) {
	timer += elapsed;
	switch (mode)
	{
	case Mode::INTRO:
		if (fire && timer>0.5f)
		{
			metrics.Restart();
			mode = Mode::GAME;
			NewGame(window);
		}
		break;
	case Mode::GAME:
		UpdateInGame(window, elapsed, fire);
		
		break;
	case Mode::ENTER_NAME:	
		if(key!=-1)
			metrics.name += key;
		if (metrics.name.size() > 1 && Keyboard::isKeyPressed(Keyboard::Return)) {
			mode = Mode::GAME_OVER;
			metrics.SortAndUpdatePlayerData();
			metrics.Save();
		}
		break;
	case Mode::GAME_OVER:
		if (fire && timer > 0.5f) {
			mode = Mode::INTRO;
			timer = 0;
		}
		break;
	default:
		assert(false);
	}

}

void Game::RenderGameOver(sf::RenderWindow & window, float elapsed) {
	Text txt("Game over press <space>", font, 50);
	FloatRect fr = txt.getGlobalBounds();
	txt.setPosition(window.getSize().x / 2.f - fr.width / 2.f, window.getSize().y - fr.height*1.2f);
	window.draw(txt);
	txt.setString("High scores");
	txt.setPosition(window.getSize().x / 2.f - fr.width / 2.f, fr.height*0.1f);
	window.draw(txt);

}
void RenderBackground(sf::RenderWindow& window)
{
	Texture background;
	LoadTexture("data/bckgd1.jpg", background);

	Sprite bg(background);
	window.draw(bg);
}
//loads the background 
void Game::Render(sf::RenderWindow & window, float elapsed) {

	switch (mode)
	{
		case Mode::INTRO:
		{
			Text txt("Legend Quest 2D 1.0\n\n     Press <space>", font, 50);
			FloatRect fr = txt.getGlobalBounds();			
			txt.setPosition(window.getSize().x/2.f - fr.width / 2.f, window.getSize().y/2.f - fr.height / 2.f);
			window.draw(txt);		
			break;
		}
		case Mode::GAME:
		{
			RenderBackground(window);//loads the background before the loop so it doesnt overlap the player 
			for (size_t i = 0; i < objects.size(); ++i)
			{
			objects[i].Render(window, elapsed);

			
				
			
			
			}		
			//particleSys.Render(window, elapsed);
			RenderHUD(window, elapsed, font);
		}
		break;
	case Mode::ENTER_NAME:
		{
			Text txt("Game over - Enter name <return>: " + metrics.name, font, 40);
			FloatRect fr = txt.getGlobalBounds();
			txt.setPosition(window.getSize().x / 2.f - fr.width / 2.f, window.getSize().y / 2.f - fr.height / 2.f);
			window.draw(txt);
			break;
		}
	case Mode::GAME_OVER:
		{
			RenderGameOver(window,elapsed);
			break;
		}
	default:
		assert(false);
	}
}

void Game::RenderHUD(sf::RenderWindow & window, float elapsed, sf::Font & font) {
}
