#pragma once

namespace geng
{

	// A simple class to describe animations
	// An animation is a sequence of values driven by time, represented by frames
	// Since the number of ticks and frames is different, the two intervals must be mapped
	// on each other.

	enum class AnimationDirection
	{
		Forward,
		Backward
	};

	// To avoid divisions, we use clever math to figure out whether the next frame tick
	// is above the next animation tick (and which one)

	struct AnimationArgs
	{
		unsigned int tickCount;
		unsigned int frameCount;
	};

	class Animation
	{
	public:
		// If 0 is passed here, the tick count will be set to "optimum resolution", one 
		// tick per frame
		bool SetArguments(const AnimationArgs& args);

		bool Step(bool forward, bool wrapAround);

		unsigned int GetCurrentTick() const { return m_currentTick; }
		unsigned int GetTotalTicks() const { return m_args.tickCount; }

		// Ticks and frames do not coincide in general, but the first tick corresponds to the first
		// frame and the last tick corresponds to the last frame
		bool IsAtStart() const { return m_currentTick == 0; }
		bool IsAtEnd() const { return m_currentTick == m_args.tickCount; }

	private:
		// Parameters
		AnimationArgs m_args;

		// Animation state
		// There is no way around keeping the current frame, as it is our unit of discretion
		// The current tick is updated accordingly
		unsigned long m_currentFrame;
		unsigned long m_currentTick;

		unsigned long m_frScaled{ 0 };
		unsigned long m_tickScaled{ 0 };
	};

}