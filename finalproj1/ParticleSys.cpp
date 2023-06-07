#include <assert.h>

#include "ParticleSys.h"
#include "Game.h"

using namespace std;
using namespace sf;

void Particles::Init() {
	LoadTexture("data/circle.png", texSprite);
	Particle p;
	p.spr.setTexture(texSprite);
	particles.clear();
	particles.insert(particles.begin(), 5000, p);
	for (size_t i = 0; i < (particles.size() - 1); ++i)
		particles[i].pNext = &particles[i + 1];
	pFree = &particles[0];
	pBusy = nullptr;
}

Particle * Particles::Remove(Particle * p, Particle * pPrev) {
	if (pPrev == nullptr)
		pBusy = p->pNext;
	else
		pPrev->pNext = p->pNext;
	Particle *pNext = p->pNext;
	p->pNext = pFree;
	pFree = p;
	return pNext;
}

void Particles::Update(float dT) {
	Particle *p = pBusy, *pPrev = nullptr;
	while (p) {
		p->life -= dT;
		if (p->life <= 0) {
			p = Remove(p, pPrev);
		}
		else if (p) {
			Vector2f pos = p->spr.getPosition();
			pos += p->vel * dT;
			p->spr.setPosition(pos);
			pPrev = p;
			p = p->pNext;
		}
	}
}

void Particles::Render(sf::RenderWindow & window) {
	Particle *p = pBusy;
	while (p) {
		window.draw(p->spr, sf::RenderStates(sf::BlendAdd));
		p = p->pNext;
	}
}

Particle * Emitter::GetNewParticle(Particles & cache) {
	Particle *p = nullptr;
	if (cache.pFree)
	{
		p = cache.pFree;
		cache.pFree = p->pNext;
		p->pNext = cache.pBusy; 
		cache.pBusy = p;
	}
	return p;
}

void Emitter::Update(float dT, Particles & cache) 
{
	if (alive) {
		lastEmit += dT;
		if (lastEmit > rate) {
			int n = numAtOnce;
			if (numAtOnce > numToEmit)
				n = numToEmit;
			while (n) {
				Particle *p = GetNewParticle(cache);
				if (p) {
					p->life = life;
					float alpha = (float)(rand() % 360);
					float speed = (float)(initSpeed.x + rand() % (initSpeed.y - initSpeed.x));
					p->vel = Vector2f(cosf(alpha), sinf(alpha)) * speed;
					p->vel += initVel;
					p->spr.setPosition(pos);
					p->spr.setColor(colour);
					p->spr.setScale(scale);
					numToEmit--;
					lastEmit = 0;
				}
				--n;   
			}
		}
		if (numToEmit <= 0) 
			alive = false;
	}
}

void ParticleSys::Init() {
	//cache.Init();
	//emitters.clear();
	//emitters.insert(emitters.begin(), 50, Emitter());
}

void ParticleSys::Update(float dT) {
	cache.Update(dT);
	for (size_t i = 0; i < emitters.size(); ++i)
		emitters[i].Update(dT, cache);
}

void ParticleSys::Render(sf::RenderWindow & window, float dT) {
	cache.Render(window);
}

Emitter *ParticleSys::GetNewEmitter() {
	Emitter *pOldest = nullptr, *pNew = nullptr;
	size_t idx = 0;

	while (idx < emitters.size() && !pNew) {
		if (!emitters[idx].alive)
			pNew = &emitters[idx];
		++idx;
	}
	if (!pNew)
		return nullptr;
	pNew->alive = true;
	return pNew;
}

int ParticleSys::GetNumActiveEmitters() const {
	int n = 0;
	for (size_t i = 0; i < emitters.size(); ++i)
		if (emitters[i].alive)
			++n;
	return n;
}
