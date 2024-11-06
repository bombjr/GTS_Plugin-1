#pragma once
#include <random>
#include "xoshiro.hpp"

namespace Gts {


	inline static std::random_device rdevice;
	inline static xso::rng32 generator(rdevice());

	// ------------------
	// Random Float
	// -----------------

	[[nodiscard]] static inline float RandomFloat(const float a_min, const float a_max) {
		std::uniform_real_distribution<> dist(a_min, a_max);
		return static_cast<float>(dist(generator));
	}

	[[nodiscard]] inline float RandomFloat() {
		std::uniform_real_distribution<> dist(0.f, std::numeric_limits<float>::max());
		return static_cast<float>(dist(generator));
	}

	// ------------------
	// Random Int
	// -----------------

	[[nodiscard]] inline int RandomInt(const int a_min, const int a_max) {
		std::uniform_int_distribution<> dist(a_min, a_max);
		return dist(generator);
	}

	[[nodiscard]] inline int RandomInt() {
		std::uniform_int_distribution<> dist(0, INT_MAX);
		return dist(generator);
	}

	//https://en.cppreference.com/w/cpp/numeric/random/discrete_distribution
	//Example RandomIntWeighted(30,1,150) -> 
	// Adds all argumetns together then calculates odds given example numbers this will have a...
	// 30 in 181 chance to return 0,
	// 1 in 181 chance to return 1
	// 150 in 181 chance to return 2
	// x in nSum chance to return n[i]
	template <typename... Args>
	[[nodiscard]] inline int RandomIntWeighted(Args... a_weights) {
		std::discrete_distribution<> dist({ a_weights... });
		return dist(generator);
	}

	// -------------------
	// Random Bool Chance
	// -------------------

	//Returns a boolean based on the percentage chance of the value being true
	//ie. RandomPercent(70) -> 70% Chance of returning true
	[[nodiscard]] inline bool RandomPercent(float a_chance) {
		std::uniform_real_distribution<> dist(0.f, 100.f);
		return dist(generator) - a_chance >= -std::numeric_limits<float>::epsilon();
	}

	// -------------------
	//  Gaussian
	// -------------------

	//https://en.cppreference.com/w/cpp/numeric/random/normal_distribution
	[[nodiscard]] inline float RandomFloatGauss(float a_mean, float a_deviation) {
		std::normal_distribution <> dist(a_mean, a_deviation);
		return static_cast<float>(dist(generator));
	}

	[[nodiscard]] inline int RandomIntGauss(int a_mean, int a_deviation) {
		std::normal_distribution <> dist(a_mean, a_deviation);
		return static_cast<int>(dist(generator));
	}
}

