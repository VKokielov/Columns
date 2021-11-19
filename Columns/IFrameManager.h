#pragma once

#include <cinttypes>
#include <memory>

namespace geng
{
	class IFrameManager;

	enum class SimQuality
	{
		Running,
		Paused,
		Stopped,
		CatchingUp
	};

	class IFrameListener
	{
	public:
		virtual void OnFrame(IFrameManager* pManager) = 0;
	};

	class IFrameAction
	{
	public:
		virtual void Act(IFrameManager* pManager) = 0;
		virtual void Release() = 0;
	};


	struct SimState
	{
		SimQuality quality;

		// Number of miliseconds per frame (1/ideal framerate)
		// This is effectively the speed of the simulation
		unsigned long timePerFrame;
		
		// Number of frames since the start of the simulation
		unsigned long frameCount;

		// Number of frames * miliseconds per frame
		unsigned long simulatedTime;
		// Miliseconds spent simulating in realtime using monotonic clock
		unsigned long actualTime;

		// Actual framerate -- number of frames per second in the last second or more
		// (if a computation took too long)
		double actualFramerate;
	};

	enum class SimResult
	{
		OK,
		InvalidFields
	};

	constexpr uint64_t FID_QUALITY = 1; // settable
	constexpr uint64_t FID_TPF = 1 << 1; // settable
	constexpr uint64_t FID_FCOUNT = 1 << 2;
	constexpr uint64_t FID_SIMTIME = 1 << 3;
	constexpr uint64_t FID_ACTTIME = 1 << 4;
	constexpr uint64_t FID_ACT_FR = 1 << 5;

	class IFrameManager
	{
	public:
		virtual void Subscribe(const std::shared_ptr<IFrameListener>& pListener, bool isSim) = 0;
		virtual void Unsubscribe(const std::shared_ptr<IFrameListener>& pListener) = 0;
		virtual SimResult UpdateSimState(const SimState& state, uint64_t fields) = 0;
		virtual SimResult GetSimState(SimState& state, uint64_t fields) const = 0;
		virtual void Simulate() = 0;
	};


}