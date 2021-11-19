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
		ApplySimChanges();

		if (m_quality == SimQuality::Stopped)
		{
			return;
		}

		++m_frameCount;
		m_msSimulatedTime += m_msPerFrame;

		curFrameTime = std::chrono::steady_clock::now();

		msForFrame = 
			std::chrono::duration_cast<std::chrono::milliseconds>(curFrameTime - m_lastFrameTime);

		// This should be positive
		m_msActualTime += (unsigned long)msForFrame.count();
		
		if (m_msActualTime - m_msLastSecondTime >= 1000)
		{
			m_actualFPS = m_frameCount - m_frameCountAtSecondSwitch;
			m_msLastSecondTime = m_msActualTime;
			m_frameCountAtSecondSwitch = m_frameCount;
		}

		// First simulate without IO until m_msSimulatedTime is at least m_msActualTime
		// NOTE:  m_msActualTime *must* be updated - otherwise we are lying!
		// If this loop executes some critical number of times, the simulation is too slow
		// to catch up with realtime at maximum speed (which will happen if many frames take
		// longer to simulate than the number of miliseconds given to a frame)

		m_lastFrameTime = curFrameTime;
		while (m_msSimulatedTime < m_msActualTime)
		{
			m_quality = SimQuality::CatchingUp;

			RunSimulation();
			ApplySimChanges();

			if (m_quality == SimQuality::Stopped)
			{
				return;
			}

			++m_frameCount;
			m_msSimulatedTime += m_msPerFrame;

			curFrameTime = std::chrono::steady_clock::now();
			msForFrame =
				std::chrono::duration_cast<std::chrono::milliseconds>(curFrameTime - m_lastFrameTime);

			m_msActualTime += (unsigned long)msForFrame.count();
			m_lastFrameTime = curFrameTime;
		}
		m_quality = SimQuality::Running;
		
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

void geng::DefaultFrameManager::Subscribe(const std::shared_ptr<IFrameListener>& pListener, bool isSim)
{
	std::vector<std::shared_ptr<IFrameListener> > listenerVector = isSim ? m_simulator : m_io;

	if (std::find(listenerVector.begin(), listenerVector.end(), pListener) == listenerVector.end())
	{
		listenerVector.emplace_back(pListener);
	}
}
void geng::DefaultFrameManager::Unsubscribe(const std::shared_ptr<IFrameListener>& pListener)
{
	auto itIO = std::find(m_io.begin(), m_io.end(), pListener);
	if (itIO != m_io.end())
	{
		m_io.erase(itIO);
	}

	auto itSim = std::find(m_simulator.begin(), m_simulator.end(), pListener);
	if (itSim != m_simulator.end())
	{
		m_simulator.erase(itSim);
	}
}
geng::SimResult geng::DefaultFrameManager::UpdateSimState(const geng::SimState& state, uint64_t fields)
{
	bool setField{ false };
	if (fields & FID_QUALITY)
	{
		setField = true;
		m_nextFields |= FID_QUALITY;
		m_nextState.quality = state.quality;
	}
	if (fields & FID_TPF)
	{
		setField = true;
		m_nextFields |= FID_TPF;
		m_nextState.timePerFrame = state.timePerFrame;
	}

	return setField ? SimResult::OK : SimResult::InvalidFields;
}

void geng::DefaultFrameManager::ApplySimChanges()
{
	if (m_nextFields & FID_QUALITY)
	{
		m_quality = m_nextState.quality;
	}

	if (m_nextFields & FID_TPF)
	{
		m_msPerFrame = m_nextState.timePerFrame;
	}

	m_nextFields = 0;
}

geng::SimResult geng::DefaultFrameManager::GetSimState(SimState& state, uint64_t fields) const
{
	if (fields & FID_ACTTIME)
	{
		state.actualTime = m_msActualTime;
		fields &= ~FID_ACTTIME;
	}

	if (fields & FID_ACT_FR)
	{
		state.actualFramerate = m_actualFPS;
		fields &= ~FID_ACT_FR;
	}

	if (fields & FID_FCOUNT)
	{
		state.frameCount = m_frameCount;
		fields &= ~FID_FCOUNT;
	}

	if (fields & FID_QUALITY)
	{
		state.quality = m_quality;
		fields &= ~FID_QUALITY;
	}

	if (fields & FID_SIMTIME)
	{
		state.simulatedTime = m_msSimulatedTime;
		fields &= ~FID_SIMTIME;
	}

	if (fields & FID_TPF)
	{
		state.timePerFrame = m_msPerFrame;
		fields &= ~FID_TPF;
	}

	return fields == 0 ? SimResult::OK : SimResult::InvalidFields;
}