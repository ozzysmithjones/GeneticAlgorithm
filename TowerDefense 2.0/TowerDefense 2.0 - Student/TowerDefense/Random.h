#pragma once
#include <stdint.h>
#include <random>


class Random
{
public:
	/**
	 * Returns a random bool (true or false).
	 */
	static bool UniformBool();

	/**
	 * Returns a random int, long, or unsigned within the range [inclusive, inclusive]
	 */
	template <typename T>
	static T UniformInt(const T min, const T max)
	{
		std::uniform_int_distribution<T> dist(min, max);
		return dist(mersenneTwister);
	}

	/**
	 * Returns a random float or double within the range [inclusive, exclusive)
	 */
	template <typename T>
	static T UniformReal(const T min, const T max)
	{
		std::uniform_real_distribution<T> dist(min, max);
		return dist(mersenneTwister);
	}

private:
	struct Constructor
	{
		Constructor();
	};

	static Constructor cons;
	static std::mt19937 mersenneTwister;
};
