#pragma once
struct Vector2
{
	int x;
	int y;

	Vector2();
	Vector2(int x, int y) : x(x), y(y) {}
};

inline Vector2 operator+(const Vector2& a, const Vector2& b)
{
	return Vector2(a.x + b.x, a.y + b.y);
}

inline Vector2 operator-(const Vector2& a, const Vector2& b)
{
	return Vector2(a.x - b.x, a.y - b.y);
}

