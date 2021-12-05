#include "Animation.h"

bool geng::Animation::SetArguments(const AnimationArgs& args)
{
	m_args = args;
	m_halfFrameCount = m_args.frameCount / 2;
	m_ticksPerFrame = m_args.tickCount / m_args.frameCount;  // Integer division!
	m_ticksPerFrameScaled = m_ticksPerFrame * m_args.frameCount;  // ==tickCount - (tickCount % frameCount)

	m_currentFrame = 0;
	m_currentTick = 0;
	m_frScaled = 0;
	m_tickScaled = 0;

	return true;
}

void geng::Animation::Force(ForceType forceType)
{
	if (forceType == ForceType::ToStart)
	{
		m_currentFrame = 0;
		m_currentTick = 0;
		m_frScaled = 0;
		m_tickScaled = 0;
	}
	else if (forceType == ForceType::ToEnd)
	{
		m_currentFrame = m_args.frameCount;
		m_currentTick = m_args.tickCount;
		m_frScaled = m_tickScaled = m_currentFrame * m_currentTick;
	}
}
bool geng::Animation::Step(bool forward, bool wrapAround)
{
	// Compute the next frame values
	unsigned long nextFrames = m_currentFrame;
	unsigned long nextScaledFrames = m_frScaled;
	bool wasWrapped{ false };

	if (forward)
	{
		if (nextFrames == m_args.frameCount)
		{
			wasWrapped = true;
			nextFrames = 0;
			nextScaledFrames = 0;
		}
		else
		{
			++nextFrames;
			nextScaledFrames += m_args.tickCount;
		}
	}
	else
	{
		if (nextFrames == 0)
		{
			wasWrapped = true;
			nextFrames = m_args.frameCount;
			nextScaledFrames = nextFrames * m_args.tickCount;
		}
		else
		{
			--nextFrames;
			nextScaledFrames -= m_args.tickCount;
		}
	}

	if (wasWrapped && !wrapAround)
	{
		// Wrap not allowed
		return false;
	}

	// Updating frames was the easy part.  Now we have to deal with ticks!
	unsigned long nextTicks = m_currentTick;
	unsigned long nextScaledTicks = m_tickScaled;

	// The wrapped case is easy
	if (wasWrapped)
	{
		if (nextFrames == 0)
		{
			nextTicks = 0;
			nextScaledTicks = 0;
		}
		else  // nextFrames == m_args.frameCount
		{
			nextTicks = m_args.tickCount;
			nextScaledTicks = nextScaledFrames;
		}
	}
	else
	{
		if (m_args.frameCount < m_args.tickCount)
		{
			// How many times do we need to add frameCount to the current value
			// so there is at most frameCount distance between?

			if (forward)
			{
				nextTicks += m_ticksPerFrame;
				nextScaledTicks += m_ticksPerFrameScaled;

				// Adjust forward?
				if ((nextScaledTicks < nextScaledFrames)
					&& (nextScaledFrames - nextScaledTicks) >= m_args.frameCount)
				{
					++nextTicks;
					nextScaledTicks += m_args.frameCount;
				}
			}
			else
			{
				nextTicks -= m_ticksPerFrame;
				nextScaledTicks -= m_ticksPerFrameScaled;

				// Adjust back?
				if ((nextScaledTicks > nextScaledFrames)
					&& (nextScaledTicks - nextScaledFrames) >= m_args.frameCount)
				{
					--nextTicks;
					nextScaledTicks -= m_args.frameCount;
				}

			}
		}

		// The new scaled frame count is "around" the new tick count
		if (forward)
		{
			unsigned long targetScaledTicks = nextScaledTicks + m_args.frameCount;
				
			if ((nextScaledFrames >= targetScaledTicks)
				|| (targetScaledTicks - nextScaledFrames <= m_halfFrameCount))
			{
				++nextTicks;
				nextScaledTicks = targetScaledTicks;
			}
		}
		else if (nextScaledTicks > 0)
		{
			unsigned long targetScaledTicks = nextScaledTicks - m_args.frameCount;
				
			if ((nextScaledFrames <= targetScaledTicks)
				|| (nextScaledFrames - targetScaledTicks <= m_halfFrameCount))
			{
				--nextTicks;
				nextScaledTicks = targetScaledTicks;
			}
		}
	}

	// Update state 
	m_currentFrame = nextFrames;
	m_currentTick = nextTicks;
	m_frScaled = nextScaledFrames;
	m_tickScaled = nextScaledTicks;
	return true;
}