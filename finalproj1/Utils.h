#pragma once
#include <string>

//dimensions in 2D that are whole numbers
struct Dim2Di
{
	int x, y;
};

//dimensions in 2D that are floating point numbers
struct Dim2Df
{
	float x, y;
};

/*
A crude attempt to control difficulty - there are many ways to do that!
Things get created or spawned at timed regular intervals
The rate at which these spawn events happen gets faster and 
faster to make the game get harder and harder.
*/
struct SpawnTimer {
	float timer=0;				//a clock to time the next spawn
	float decayTimer = 0;		//a clock to time when the spawns should get faster

	float delay=1, originalDelay=1;	//how long to wait before the next spawn
	float decayDelay = 0;			//how long to wait before the spawning gets faster
	float decayMultiplier = 0.99f;  //when spawning gets faster, by what percentage does the delay fall

	/*
	Reset the clocks, we can do that by specifying all new delay/decay 
	time values, or leave that blank, it will remember the old values
	and just restart the timers.
	*/
	void Reset(float _delay = -1, float _decayDelay = -1, float _decayMultiplier = -1);
	/*
	Call this every updated to keep the clock ticking
	If it returns true then you need to spawn something. Note that
	this will keep returning true, once the time is right, until you
	reset it. Like an alarm that goes off at set time intervals, but
	once it does off, it doesn't stop until you reset it.
	*/
	bool Cycle(float elapsed);
};

/*
Send text to the debug output window
Second parameter can be ignored
*/
void DebugPrint(const std::string& mssg1, const std::string& mssg2 = "");

