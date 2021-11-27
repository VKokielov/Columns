#pragma once

namespace geng
{

	// A simple class to describe animations
	// An animation is a sequence of values driven by time
	// Normally the number of available values (ticks) and the total time needed
	// is known, but the relationship of ticks to frames is computed.
	// This class tries to abstract all these considerations away

	enum class AnimationDirection
	{
		Forward,
		Backward
	};

	// To avoid divisions, we use clever math to figure out whether the next frame tick
	// is above the next animation tick (and which one)

	class Animation
	{
	public:

		void SetMsPerFrame(unsigned int tpf);

		// If 0 is passed here, the tick count will be set to "optimum resolution", one 
		// tick per frame
		void SetTickCount(unsigned int tickCount);

		void SetTotalMs(unsigned int totalMs);
		void SetDirection(AnimationDirection dir);
		void SetShouldCycle(bool shouldCycle);


	private:
		// Parameters
		unsigned int m_timePerFrame;
		unsigned int m_tickCount;
		bool m_tickPerFrame;

		unsigned int m_totalMs;
		unsigned int m_frameCount;

		// Animation state
		unsigned int m_currentTick;
		


	};

}