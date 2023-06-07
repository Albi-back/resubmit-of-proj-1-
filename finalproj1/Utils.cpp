#include <windows.h>

#include "Utils.h"

using namespace std;

void DebugPrint(const string& mssg1, const string& mssg2)
{
	OutputDebugString(mssg1.c_str());
	OutputDebugString("\n");
	if (!mssg2.empty()) {
		OutputDebugString(mssg2.c_str());
		OutputDebugString("\n");
	}
}

void SpawnTimer::Reset(float _delay, float _decayDelay, float _decayMultiplier) {
	timer = 0;
	decayTimer = 0;
	delay = originalDelay;
	if (_delay != -1)
		originalDelay = delay = _delay;
	if (_decayDelay != -1) 
		decayDelay = _decayDelay;
}

bool SpawnTimer::Cycle(float elapsed) {
	timer += elapsed;
	decayTimer += elapsed;
	if (decayDelay > 0 && decayTimer >= decayDelay) {
		decayTimer = 0;
		delay *= 0.99f;
	}
	return (timer >= delay);
}
