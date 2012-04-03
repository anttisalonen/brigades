#ifndef CLOCK_H
#define CLOCK_H

class Clock {
	public:
		Clock();
		void limitFPS(int fps);
	private:
		float getTime() const;
		float mLastTime;
};

#endif
