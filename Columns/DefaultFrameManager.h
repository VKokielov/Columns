#pragma once

#include "IFrameManager.h"
#include <chrono>
#include <vector>
#include <memory>

namespace geng
{

	class DefaultFrameManager : public IFrameManager
	{
	public:
		void Simulate() override;

	private:
		// Go through the simulators and execute any actions they add
		void RunSimulation();
		// Go through the IO (rendering/input)
		void RunIO();

		// This is the sum of all the time segments actually taken per frame
		unsigned long m_msActualTime{ 0 };
		// Time of the last frame
		std::chrono::time_point<std::chrono::steady_clock>
			m_lastFrameTime;

		// Number of frames
		unsigned long m_frameCount{ 0 };
		unsigned long m_msSimulatedTime{ 0 };
		// Time per frame
		unsigned long m_msPerFrame{ 0 };
		// If actual time is less than m_msBreather behind simulated time there is no wait
		unsigned long m_msBreather;

		std::vector<std::shared_ptr<IFrameListener> >  m_simulator;
		std::vector<std::shared_ptr<IFrameAction> > m_actions;

		std::vector<std::shared_ptr<IFrameListener> >  m_io;

	};
}