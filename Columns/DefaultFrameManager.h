#pragma once

#include "IFrameManager.h"
#include <chrono>
#include <vector>
#include <memory>

namespace geng
{
	struct FrameMgrArgs
	{
		unsigned long msBreather;
		unsigned long msTimePerFrame;
		unsigned long maxMsPerFrame; // For monitoring
	};

	class DefaultFrameManager : public IFrameManager
	{
	public:
		DefaultFrameManager(const FrameMgrArgs& args);

		void Simulate() override;

		void Subscribe(const std::shared_ptr<IFrameListener>& pListener, bool isSim) override;
		void Unsubscribe(const std::shared_ptr<IFrameListener>& pListener) override;
		SimResult UpdateSimState(const SimState& state, uint64_t fields) override;
		SimResult GetSimState(SimState& state, uint64_t fields) const override;

	private:
		void ApplySimChanges();

		// Go through the simulators and execute any actions they add
		void RunSimulation();
		// Go through the IO (rendering/input)
		void RunIO();

		SimQuality m_quality{ SimQuality::Running };

		// This is the sum of all the time segments actually taken per frame
		unsigned long m_msActualTime{ 0 };
		// Time of the last frame
		std::chrono::time_point<std::chrono::steady_clock>
			m_lastFrameTime;

		// Number of frames
		unsigned long m_frameCount{ 0 };
		unsigned long m_frameCountAtSecondSwitch{ 0 };
		unsigned long m_actualFPS{ 0 };
		unsigned long m_msLastSecondTime{ 0 };

		unsigned long m_msSimulatedTime{ 0 };
		// Time per frame
		unsigned long m_msPerFrame{ 0 };
		// If actual time is less than m_msBreather behind simulated time there is no wait
		unsigned long m_msBreather;

		std::vector<std::shared_ptr<IFrameListener> >  m_simulator;
		std::vector<std::shared_ptr<IFrameAction> > m_actions;

		std::vector<std::shared_ptr<IFrameListener> >  m_io;

		uint64_t m_nextFields{ 0 };
		SimState m_nextState;
	};
}