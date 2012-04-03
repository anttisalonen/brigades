#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>

#include "Clock.h"

Clock::Clock()
{
	mLastTime = getTime();
}

void Clock::limitFPS(int fps)
{
	float newtime = getTime();
	float maxadv = 1.0f / fps;
	float diff = newtime - mLastTime;
	if(maxadv > diff) {
		usleep((maxadv - diff) * 1000000);
		mLastTime = getTime();
	}
	else {
		mLastTime = newtime;
	}
}

float Clock::getTime() const
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + tv.tv_usec / 1000000.0f;
}


