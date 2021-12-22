#pragma once

#include "IGame.h"

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <array>
#include <chrono>

namespace geng
{
	struct DefaultGameArgs : public GameArgs
	{
		unsigned long msBreather;
		unsigned long maxMsPerFrame; // For monitoring
	};

	class ListenerGroup
	{
	private:
		struct Listener_
		{
			ListenerID  lid;
			std::shared_ptr<IGameListener>   pListener;

			Listener_(ListenerID lid_, 
				const std::shared_ptr<IGameListener>& pListener_)
				:lid(lid_),
				pListener(pListener_)
			{

			}
		};
	public:
		void OnFrame(const SimState& rState, const SimContextState* pCtxState) const;
		bool AddListener(ListenerID lid, const std::shared_ptr<IGameListener>& pListener);
	private:
		std::vector<Listener_>   m_listeners;
	};

	class ListenerTypeList
	{
		// A sort of compact linked list
	private:
		struct ListenerTypeEntry_
		{
			ContextID  contextId;
			ListenerGroup group;
			size_t nextIndex;
			size_t prevIndex;

			ListenerTypeEntry_(ContextID contextId_, size_t nextIndex_, size_t prevIndex_)
				:contextId(contextId_), 
				nextIndex(nextIndex_),
				prevIndex(prevIndex_)
			{ }
		};

		struct ContextIndexRequest
		{
			ContextID contextId;
			bool ctxFound{ false };
			size_t ctxIdx;

			ContextIndexRequest(ContextID ctxNeeded)
				:contextId(ctxNeeded)
			{ }
		};
	public:
		// Add a new context
		// NOTE:  We *stipulate* that the context id will be identical
		//  with the vector index here!  This prevents needless seeking and mapping.

		bool AddContext(ContextID contextId);
		// Move a context to the end of the list
		bool MoveToFront(ContextID contextId);
		// Move a context to the beginning of the list
		bool SendToBack(ContextID contextId);
		bool MakePredecessor(ContextID contextId, ContextID successorId);

		// Iterate in order through all contexts from first to last
		template<typename F>
		void IterContexts(F&& callback)
		{
			size_t idx = m_firstIndex;

			while (idx != m_lastIndex)
			{
				if (idx != EXECUTIVE_CONTEXT)
				{
					callback(m_listenerGroups[idx].contextId, m_listenerGroups[idx].group);
				}
				idx = m_listenerGroups[idx].nextIndex;
			}

			// One more step for "last"
			callback(m_listenerGroups[idx].contextId, m_listenerGroups[idx].group);

		}

		bool AddListener(ContextID contextId,
			ListenerID lid,
			const std::shared_ptr<IGameListener>& pListener);
	private:
		size_t m_firstIndex;
		size_t m_lastIndex;
		std::vector<ListenerTypeEntry_> m_listenerGroups;
	};

	class DefaultGame : public IGame, 
		public std::enable_shared_from_this<DefaultGame>
	{
	private:
		struct Context_
		{
			std::string name;
			SimContextState contextState;

			bool m_nextFocus{ true };
			bool m_nextRun{ true };
			bool m_nextVisible{ true };

			Context_(const char* pName)
				:name(pName)
			{ }
		};
	public:
		static std::shared_ptr<DefaultGame> CreateGame(const DefaultGameArgs& args);

		bool AddComponent(const std::shared_ptr<IGameComponent>& pComponent) override;

		const std::shared_ptr<IGameComponent>& GetComponent(const char* pName) override;
		bool Run() override;
		void Quit() override;
		void LogError(const char* pError) override;

		ContextID CreateSimContext(const char* pName) override;
		ContextID GetSimContext(const char* pName) const override;

		bool AddListener(ListenerType listenerType,
			ContextID contextId,
			const std::shared_ptr<IGameListener>& pListener,
			ListenerID* plistenerId) override;

		bool MoveToFront(ListenerType type, ContextID contextId) override;
		bool SendToBack(ListenerType type, ContextID contextId) override;
		bool MakePredecessor(ListenerType type, ContextID contextId,
			ContextID successorId) override;

		bool SetVisibility(ContextID contextId, bool value) override;
		bool SetRunState(ContextID contextId, bool value) override;
		bool SetFocus(ContextID contextId) override;
		bool SetFrameIndex(ContextID contextId, unsigned long frameCount = 0) override;

		const GameArgs& GetGameArgs() const override;

	private:
		DefaultGame(const DefaultGameArgs& args);

		ListenerTypeList* SelectListenerList(ListenerType listenerType)
		{
			ListenerTypeList* pList = nullptr;

			switch (listenerType)
			{
			case ListenerType::Input:
				pList = &m_inputList;
				break;
			case ListenerType::Simulation:
				pList = &m_simList;
				break;
			case ListenerType::Rendering:
				pList = &m_renderList;
				break;
			}

			return pList;
		}

		void RunGameLoop();
		void UpdateContextStateBefore();
		void ContextInputCallbacks();
		void ContextSimCallbacks();
		void ContextRenderCallbacks();
		void UpdateContextStateAfter();


		std::unordered_map<std::string, size_t>
			m_componentMap;

		std::vector<std::shared_ptr<IGameComponent> > m_components;

		DefaultGameArgs m_gameArgs;

		// One entry per context
		SimState m_simState;
		std::vector<Context_>  m_contexts;
		ListenerTypeList m_inputList;
		ListenerTypeList m_simList;
		ListenerTypeList m_renderList;

		// Executive listeners
		ListenerGroup m_executiveListeners;

		bool m_isActive{ false };
		ContextID m_focus{ 0 };

		ListenerID  m_listenerID{ 0 };

		// Time of the last frame
		unsigned long m_msActualTime{ 0 };
		std::chrono::time_point<std::chrono::steady_clock>
			m_lastFrameTime;
		unsigned long m_frameCountAtSecondSwitch{ 0 };
		unsigned long m_actualFPS{ 0 };
		unsigned long m_msLastSecondTime{ 0 };

		// This flag indicates that executive listeners are executing and that run state, focus, etc
		// changes should be applied to *this* frame and not the next one
		bool m_callingExecutive{ false };
	};

}