#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>

#include "Clock.h"

Clock::Clock()
	: mFrames(0)
{
	mLastTime = getTime();
	mStatTime = mLastTime;
}

void Clock::limitFPS(int fps)
{
	double newtime = getTime();
	double maxadv = 1.0f / fps;
	double diff = newtime - mLastTime;
	if(maxadv > diff) {
		usleep((maxadv - diff) * 1000000);
		mLastTime = getTime();
	}
	else {
		mLastTime = newtime;
	}
	mFrames++;
	if(newtime - mStatTime >= 2.0f) {
		std::cout << "FPS: " << mFrames / (newtime - mStatTime) << "\n";
		mStatTime = newtime;
		mFrames = 0;
	}
}

double Clock::getTime() const
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double)tv.tv_sec + (double)(tv.tv_usec / 1000000.0f);
}


