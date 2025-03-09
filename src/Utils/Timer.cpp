#include "Utils/Timer.hpp"

using namespace GTS;

namespace {

	inline double GetGameTime() {
		return Time::WorldTimeElapsed();
	}

	inline std::uint64_t GetGameFrame() {
		return Time::FramesElapsed();
	}
}

namespace GTS {

	Timer::Timer(double delta) : delta(delta) {}

	bool Timer::ShouldRun() {

		if (this->delta == 0.0) return false;

		double currentTime = GetGameTime();

		if (this->last_time + this->delta <= currentTime) {

			this->elaped_time = currentTime - this->last_time;

			std::uint64_t currentFrame = GetGameFrame();

			this->elaped_frame = currentFrame - this->last_frame;

			this->Reset();
			return true;
		}
		return false;
	}

	void Timer::UpdateDelta(const float a_delta) {
		this->delta = a_delta;
	}

	double Timer::TimeDelta() const {
		return this->elaped_time;
	}

	bool Timer::ShouldRunFrame() {

		if (this->delta == 0.0) return false;

		std::uint64_t currentFrame = GetGameFrame();

		if (Timer::ShouldRun()) {
			return true;
		}
		else if (currentFrame == this->last_frame) {
			return true;
		}
		else {
			return false;
		}
	}

	void Timer::Reset() {
		this->last_time = GetGameTime();
		this->last_frame = GetGameFrame();
	}

	std::uint64_t Timer::FrameDelta() const {
		return this->elaped_frame;
	}
}
