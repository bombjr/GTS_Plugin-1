#pragma once

#include "ClibUtil/rng.hpp"

namespace GTS {

	inline static std::random_device rdevice;
	//inline static XoshiroCpp::Xoshiro256StarStar generator(rdevice());
	inline static clib_util::RNG generator(rdevice());

	// ------------------
	// Random Float
	// -----------------

	[[nodiscard]] static inline float RandomFloat(float a_min, float a_max) {
		// If min > max, swap them
		if (a_min > a_max) {
			std::swap(a_min, a_max);
		}

		std::uniform_real_distribution<> dist(a_min, a_max);
		return static_cast<float>(dist(generator));
	}

	[[nodiscard]] static inline float RandomFloat() {
		std::uniform_real_distribution<> dist(0.f, std::numeric_limits<float>::max());
		return static_cast<float>(dist(generator));
	}

	// ------------------
	// Random Int
	// -----------------

	[[nodiscard]] static inline int RandomInt(int a_min, int a_max) {

		// If min > max, swap them
		if (a_min > a_max) {
			std::swap(a_min, a_max);
		}

		std::uniform_int_distribution<> dist(a_min, a_max);
		return dist(generator);
	}

	[[nodiscard]] static inline int RandomInt() {
		std::uniform_int_distribution<> dist(0, INT_MAX);
		return dist(generator);
	}

    [[nodiscard]] static inline int _RandomIntWeighted(std::initializer_list<int> a_weights) {
        std::discrete_distribution<> dist(a_weights.begin(), a_weights.end());
        return dist(generator);
    }

	[[nodiscard]] static inline int RandomIntWeighted(std::vector<int> a_weights) {
		std::discrete_distribution<> dist(a_weights.begin(), a_weights.end());
		return dist(generator);
	}


	template <std::size_t N>
	[[nodiscard]] static inline int RandomIntWeighted(const std::array<int, N>& a_weights) {
		std::discrete_distribution<> dist(a_weights.begin(), a_weights.end());
		return dist(generator);
	}

	//https://en.cppreference.com/w/cpp/numeric/random/discrete_distribution
	//Example RandomIntWeighted(30,1,150) -> 
	// Adds all argumetns together then calculates odds given example numbers this will have a...
	// 30 in 181 chance to return 0,
	// 1 in 181 chance to return 1
	// 150 in 181 chance to return 2
	/// x in nSum chance to return n[i]
    template <typename... Args>
    [[nodiscard]] static inline int RandomIntWeighted(Args... a_weights) {
        return _RandomIntWeighted({ a_weights... });
    }

	// -------------------
	// Random Bool Chance
	// -------------------

	/// <summary>
	/// Use Bernouli distribution to generate a random boolean
	/// </summary>
	/// <param name="a_trueChance">0-100 chance for the value to be true, 0 is always false 100 is always true</param>
	/// <returns>true or false</returns>
	[[nodiscard]] static inline int RandomBool(const float a_trueChance = 50.0f) {
		const float probability = a_trueChance / 100.0f;
		std::bernoulli_distribution dist(probability);
		return dist(generator);
	}

	// -------------------
	//  Gaussian
	// -------------------

	//https://en.cppreference.com/w/cpp/numeric/random/normal_distribution
	[[nodiscard]] static inline float RandomFloatGauss(float a_mean, float a_deviation) {
		std::normal_distribution <> dist(a_mean, a_deviation);
		return static_cast<float>(dist(generator));
	}

	[[nodiscard]] static inline int RandomIntGauss(int a_mean, int a_deviation) {
		std::normal_distribution <> dist(a_mean, a_deviation);
		return static_cast<int>(dist(generator));
	}
}

