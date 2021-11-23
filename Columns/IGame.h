#pragma once

#include <memory>

namespace geng
{

	enum class ListenerType
	{
		Executive,
		Input,
		Simulation,
		Rendering
	};
	
	class IGame;

	using ContextID = size_t;
	using ListenerID = size_t;
	constexpr ContextID EXECUTIVE_CONTEXT = 0;

	struct SimState
	{
		// Number of miliseconds per frame (1/ideal framerate)
		// This is effectively the speed of the simulation
		unsigned long timePerFrame{ 0 };

		// Miliseconds spent simulating in realtime using monotonic clock
		unsigned long actualTime{ 0 };

		// Actual framerate -- number of frames per second in the last second or more
		// (if a computation took too long)
		double actualFramerate{ 0.0 };

		// "Executive" sim time always runs regardless of the state of a context
		// and is used to decide whether we are "behind" in execution
		unsigned long execFrameCount{ 0 };
		unsigned long execSimulatedTime{ 0 };

		bool catchingUp{ false };
	};

	struct ContextFlag
	{
		bool prevValue{ false };
		bool curValue{ false };
	};

	struct SimContextState
	{
		// Number of frames since the start of the simulation
		unsigned long frameCount{ 0 };

		// Number of frames * miliseconds per frame
		unsigned long simulatedTime{ 0 };
		
		// Is the context in focus?
		ContextFlag focus{};
		// Is the context visible/rendered?
		ContextFlag visibility{};
		// Is the context running or stopped/paused?
		ContextFlag runstate{};
	};

	class IGameComponent
	{
	public:
		virtual const char* GetName() const = 0;
		virtual ~IGameComponent() = default;

		virtual bool Initialize(const std::shared_ptr<IGame>& pGame) = 0;
		virtual void WindDown(const std::shared_ptr<IGame>& pGame) = 0;
	};

	class IGameListener : public IGameComponent
	{
	public:
		// For "executive" listeners, the second value is nullptr
		virtual void OnFrame(const SimState& rSimState,
			const SimContextState* pContextState) = 0;
	};


	class IGame
	{
	public:
		virtual ~IGame() = default;

		virtual bool AddComponent(const std::shared_ptr<IGameComponent>& pComponent) = 0;
		virtual const std::shared_ptr<IGameComponent>&
			GetComponent(const char* pName) = 0;

		virtual void LogError(const char* pError) = 0;
		virtual bool Run() = 0;

		virtual void Quit() = 0;

		// Executive functions for managing contexts 
		virtual ContextID CreateSimContext(const char* pName) = 0;

		// Listener manipulation
		virtual bool AddListener(ListenerType listenerType,
			ContextID contextId,
			const std::shared_ptr<IGameListener>& pListener,
			ListenerID* plistenerId = nullptr) = 0;

		// Move the listener group to the front of the listener queue among all other contexts
		// for a given listener type
		virtual bool MoveToFront(ListenerType type, ContextID contextId) = 0;
		virtual bool SendToBack(ListenerType type, ContextID contextId) = 0;
		virtual bool MakePredecessor(ListenerType type, ContextID contextId, 
									 ContextID successorId) = 0;

		virtual bool SetVisibility(ContextID contextId, bool value) = 0;
		virtual bool SetRunState(ContextID contextId, bool value) = 0;
		virtual bool SetFocus(ContextID contextId) = 0;
	};

}