#include "DefaultFrameManager.h"

#include <thread>

void geng::DefaultFrameManager::RunIO()
{
	for (size_t i = 0; i < m_io.size(); ++i)
	{
		m_io[i]->OnFrame(this);
	}
}

void geng::DefaultFrameManager::RunSimulation()
{
	for (size_t i = 0; i < m_simulator.size(); ++i)
	{
		m_simulator[i]->OnFrame(this);
	}

	// Now execute the actions
	for (size_t i = 0; i < m_actions.size(); ++i)
	{
		m_actions[i]->Act(this);
		m_actions[i]->Release();
	}
}

void geng::DefaultFrameManager::Simulate()
{
	// 60 FPS is 0.06 frames in a millisecond.  That's ~17 milliseconds per frame

	bool isActive{ true };

	m_lastFrameTime = std::chrono::steady_clock::now();

	std::chrono::milliseconds msForFrame;
	std::chrono::time_point<std::chrono::steady_clock> curFrameTime;
	while (isActive)
	{
		RunSimulation();
		RunIO();
		++m_frameCount;
		m_msSimulatedTime += m_msPerFrame;

		curFrameTime = std::chrono::steady_clock::now();

		msForFrame = 
			std::chrono::duration_cast<std::chrono::milliseconds>(curFrameTime - m_lastFrameTime);

		m_msActualTime += msForFrame.count();

		// First simulate without IO until m_msSimulatedTime is at least m_msActualTime
		// NOTE:  m_msActualTime *must* be updated - otherwise we are lying!
		// If this loop executes some critical number of times, the simulation is too slow
		// to catch up with realtime at maximum speed (which will happen if many frames take
		// longer to simulate than the number of miliseconds given to a frame)

		m_lastFrameTime = curFrameTime;
		while (m_msSimulatedTime < m_msActualTime)
		{
			RunSimulation();
			++m_frameCount;
			m_msSimulatedTime += m_msPerFrame;

			curFrameTime = std::chrono::steady_clock::now();
			msForFrame =
				std::chrono::duration_cast<std::chrono::milliseconds>(curFrameTime - m_lastFrameTime);

			m_msActualTime += msForFrame.count();
			m_lastFrameTime = curFrameTime;
		}
		
		// What happens next depends on the relation between actual and simulated time
		if (m_msActualTime < m_msSimulatedTime)
		{
			// Possibly wait
			if (m_msSimulatedTime - m_msActualTime > m_msBreather)
			{
				unsigned long msSleep = m_msSimulatedTime - m_msActualTime - m_msBreather;
				std::this_thread::sleep_for(std::chrono::milliseconds(msSleep));
			}

			// The breather is silently ignored
			m_msActualTime = m_msSimulatedTime;
		}
	}


}