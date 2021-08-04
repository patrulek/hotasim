#include <random>

#include "structures.h"

int getRandomInt(int _min, int _max) {
	 static std::random_device rd;  //Will be used to obtain a seed for the random number engine
	 static std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	 std::uniform_int_distribution<> distrib(_min, _max);

	 return distrib(gen);
}

float getRandomFloat(float _min, float _max) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<float> distrib(_min, _max);

	return distrib(gen);
}