#ifndef TERRAIN_H
#define TERRAIN_H

#include <iostream>
#include <noise/noise.h>

struct Vector2 {
	float x;
	float y;
	Vector2() : x(0), y(0) { }
	Vector2(float x_, float y_) : x(x_), y(y_) { }
	Vector2 operator+(const Vector2& v2) const;
	Vector2 operator-(const Vector2& v2) const;
	float length() const;
	float length2() const;
	void normalize();
	Vector2 normalized() const;
	void operator+=(const Vector2& v2);
	void operator*=(float f);
};

std::ostream& operator<<(std::ostream& out, const Vector2& vec);

struct Area2 {
	float x1, y1, x2, y2;
	Area2() : x1(0), y1(0), x2(0), y2(0) { }
	Area2(float x1_, float y1_, float x2_, float y2_)
		: x1(x1_), y1(y1_), x2(x2_), y2(y2_) { }
};

class Terrain {
	public:
		Terrain();
		float getHeightAt(const Vector2& v) const;
		float getVegetationAt(const Vector2& v) const;
		float getHeightScale() const;
		float getWidth() const;
	private:
		noise::module::Perlin mPerlin;
		float mHeightScale;
		float mHeightPlaneScale;
		float mVegetationScale;
};



#endif

