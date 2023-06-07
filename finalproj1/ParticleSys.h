#pragma once
#include <vector>

#include "SFML/Graphics.hpp"
#include "Utils.h"

/*
A tiny sprite flyign through the game world with a limited lifespan
*/
struct Particle {
	Particle *pNext = nullptr;  //this is an element in a linked list, so we need a pointer to the item (or null)
	sf::Sprite spr;		//2d sprite which also holds an XY position
	float life = 0;		//seconds to live, zero means dead
	sf::Vector2f vel;	//velocity is direction and speed
};

/*
We need lots of particles but we don't want to call slow new/delete to make them
Preallocate a vector array of thousands, but connect them up into linked lists
*/
struct Particles {
	std::vector<Particle> particles;			//thousands of particles, never delete or add to this once it's setup
	Particle *pBusy = nullptr;//pBusy linked list connects alive particles
	Particle *pFree = nullptr;//pFree linked list connects dead particles to be used again when needed
	sf::Texture texSprite;		//a texture with the particle image on it for all the particle sprites

	//setup thousands of particles ONCE
	void Init();
	//remove a particle from the busy list and put it in the free list to use again
	Particle *Remove(Particle *p, Particle *pPrev);
	//run physics on all particles that are alive
	void Update(float dT);
	//are any particles alive?
	bool IsBusy() const {
		return pBusy!=nullptr;
	}
	//render every alive particle
	void Render(sf::RenderWindow& window);
};

/*
Emit particles in a controlled way at a specific position
E.g. when an explosion occurs, create an emitter at that position
and let it emit 500 particles
*/
struct Emitter {
	sf::Vector2f pos;					//where in the game world
	sf::Vector2f scale{ 1,1 };			//min and max scale, pick randomly
	sf::Vector2f initVel{ 0,0 };		//emitter could be moving
	Dim2Di initSpeed;					//min and max speed, pick randomly
	float rate = 0.01f;					//how often to fire off particles
	sf::Color colour = sf::Color::White;//what colour tint on the particles
	float life = 1.f;		//how long do particles live
	int numToEmit = 0;		//how many in total does the emitter have to emit
	int numAtOnce = 1;		//how many to fire off at one
	bool alive = false;		//is this emitter active

	float lastEmit = 0;		//when did we emit last in seconds

	/*
	See if there are any particles in the cache, does pFree point at anything?
	If there is a free one then move it to the pBusy list
	*/
	Particle* GetNewParticle(Particles& cache);
	/*
	Give the emitter the chance to emit a particle
	*/
	void Update(float dT, Particles& cache);
};

/*
One object containing a cache of thousands of particles and
a cache of Emitters to use in firing the particles off
*/
struct ParticleSys {
	Particles cache;				//thousands of particles
	std::vector<Emitter> emitters;	//multiple emitters we can reuse for particle firing

	/*
	One time setup
	*/
	void Init();
	//let any alive emitters update
	void Update(float dT);
	//render all busy list particles
	void Render(sf::RenderWindow& window, float dT);
	//see if there is a dead emitter we can reuse
	//could return a nullptr - meaning none available yet
	Emitter* GetNewEmitter();
	//how many emitters are firing?
	int GetNumActiveEmitters() const;
};

