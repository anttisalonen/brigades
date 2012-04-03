#ifndef CLOCK_H
#define CLOCK_H

class Clock {
	public:
		Clock();
		void limitFPS(int fps);
	private:
		double getTime() const;
		double mLastTime;
		double mStatTime;
		int mFrames;
};

#endif
