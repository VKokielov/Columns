#include "DefaultGame.h"

#include <iostream>
#include <chrono>
#include <thread>

void geng::ListenerGroup::OnFrame(const SimState& rState, const SimContextState* pCtxState) const
{
	for (const Listener_& listener : m_listeners)
	{
		listener.pListener->OnFrame(rState, pCtxState);
	}
}
bool geng::ListenerGroup::AddListener(ListenerID lid, const std::shared_ptr<IGameListener>& pListener)
{
	m_listeners.emplace_back(lid, pListener);
	return true;
}

bool geng::ListenerTypeList::AddContext(ContextID contextId)
{
	if (contextId != m_listenerGroups.size())
	{
		return false;
	}

	// Add and update the next index
	size_t newLast = m_listenerGroups.size();
	size_t curLast = 0;
	if (m_listenerGroups.empty())
	{
		m_firstIndex = 0;
		m_lastIndex = 0;
	}
	else
	{
		// add at the end
		curLast = m_lastIndex;
		m_listenerGroups[m_lastIndex].nextIndex = newLast;
		m_lastIndex = newLast;
	}
	m_listenerGroups.emplace_back(contextId, 0, curLast);

	return true;
}

bool geng::ListenerTypeList::MoveToFront(ContextID contextId)
{
	// Context has been found.  Move it to the front by jiggling the pointers
	if (m_lastIndex == contextId)
	{
		return true;
	}
	else if (m_firstIndex == contextId) // More than one, last != first
	{
		m_firstIndex = m_listenerGroups[contextId].nextIndex;
	}
	else  // In the middle
	{
		auto prevIdx = m_listenerGroups[contextId].prevIndex;
		auto nextIdx = m_listenerGroups[contextId].nextIndex;
		// Link prev with next and back
		m_listenerGroups[prevIdx].nextIndex = nextIdx;
		m_listenerGroups[nextIdx].prevIndex = prevIdx;
	}

	m_listenerGroups[m_lastIndex].nextIndex = contextId;
	m_listenerGroups[contextId].prevIndex = m_lastIndex;
	m_lastIndex = contextId;
	return true;
}

bool geng::ListenerTypeList::SendToBack(ContextID contextId)
{
	// Context has been found.  Move it to the front by jiggling the pointers
	if (m_lastIndex == contextId)
	{
		return true;
	}
	else if (m_firstIndex == contextId) // More than one, last != first
	{
		m_lastIndex = m_listenerGroups[contextId].prevIndex;
	}
	else  // In the middle
	{
		auto prevIdx = m_listenerGroups[contextId].prevIndex;
		auto nextIdx = m_listenerGroups[contextId].nextIndex;
		// Link prev with next and back
		m_listenerGroups[prevIdx].nextIndex = nextIdx;
		m_listenerGroups[nextIdx].prevIndex = prevIdx;
	}

	m_listenerGroups[m_firstIndex].prevIndex = contextId;
	m_listenerGroups[contextId].nextIndex = m_firstIndex;
	m_firstIndex = contextId;

	return true;
}


bool geng::ListenerTypeList::MakePredecessor(ContextID contextId, ContextID successorId)
{
	// Obviously if there's only one value this will be true
	if (successorId == m_firstIndex)
	{
		return SendToBack(contextId);
	}

	if (m_lastIndex != contextId && m_listenerGroups[contextId].nextIndex == successorId)
	{
		return true;
	}

	// We now know that the successor has a valid predecessor.  Excise
	// Excising is a bit tricky 
	bool hasPred = m_firstIndex != contextId;
	bool hasSucc = m_lastIndex != contextId;

	// Both cannot be false per the first test above

	if (hasPred)
	{
		auto prevIdx = m_listenerGroups[contextId].prevIndex;
		if (hasSucc) 
		{
			// both successor and predecessor
			auto nextIdx = m_listenerGroups[contextId].nextIndex;

			m_listenerGroups[prevIdx].nextIndex = nextIdx;
			m_listenerGroups[nextIdx].prevIndex = prevIdx;
		}
		else  // predecessor but no successor
		{
			m_lastIndex = prevIdx;
		}
	}
	else  // Successor but no predecessor
	{
		m_firstIndex = m_listenerGroups[contextId].nextIndex;
	}
	
	// Now paste it into its new surroundings, knowing that it has both a successor (known)
	// and a predecessor (first condition)

	auto newPrevIndex = m_listenerGroups[successorId].prevIndex;

	m_listenerGroups[successorId].prevIndex = contextId;
	m_listenerGroups[newPrevIndex].nextIndex = contextId;
	m_listenerGroups[contextId].nextIndex = successorId;
	m_listenerGroups[contextId].prevIndex = newPrevIndex;
	return true;
}

bool geng::ListenerTypeList::AddListener(ContextID contextId,
	ListenerID lid,
	const std::shared_ptr<IGameListener>& pListener)
{
	return m_listenerGroups[contextId].group.AddListener(lid, pListener);
}

geng::DefaultGame::DefaultGame(const DefaultGameArgs& args)
	:m_gameArgs(args)
{
	// Add the executive context (just a placeholder)
	CreateSimContext("ExecutiveContext");
}

std::shared_ptr<geng::DefaultGame> geng::DefaultGame::CreateGame(const DefaultGameArgs& args)
{
	return std::shared_ptr<geng::DefaultGame>(new DefaultGame(args));
}

bool geng::DefaultGame::AddComponent(const std::shared_ptr<IGameComponent>& pComponent)
{
	const char* pName = pComponent->GetName();

	// Try to add
	std::string sname{ pName };
	if (m_componentMap.count(sname) != 0)
	{
		return false;
	}

	m_componentMap.emplace(sname, m_components.size());
	m_components.emplace_back(pComponent);

	return true;
}

geng::ContextID geng::DefaultGame::CreateSimContext(const char* pName)
{
	// Make sure it's not in the set
	std::string sname{ pName };
	for (size_t i = 0; i < m_contexts.size(); ++i)
	{
		if (m_contexts[i].name == sname)
		{
			return EXECUTIVE_CONTEXT;
		}
	}

	ContextID nextContext = m_contexts.size();
	m_contexts.emplace_back(pName);
	m_inputList.AddContext(nextContext);
	m_simList.AddContext(nextContext);
	m_renderList.AddContext(nextContext);

	return nextContext;
}

geng::ContextID geng::DefaultGame::GetSimContext(const char* pName) const
{
	std::string sname{ pName };
	for (size_t i = 0; i < m_contexts.size(); ++i)
	{
		if (m_contexts[i].name == sname)
		{
			return i;
		}
	}

	return EXECUTIVE_CONTEXT;
}

bool geng::DefaultGame::AddListener(ListenerType listenerType,
	ContextID contextId,
	const std::shared_ptr<IGameListener>& pListener,
	ListenerID* plistenerId)
{
	if (listenerType == ListenerType::Executive)
	{
		contextId = EXECUTIVE_CONTEXT;
	}

	if (contextId >= m_contexts.size())
	{
		return false;
	}

	// Special handling for executive listener
	if (contextId == EXECUTIVE_CONTEXT)
	{
		ListenerID lid = m_listenerID++;
		if (plistenerId)
		{
			*plistenerId = lid;
		}
		return m_executiveListeners.AddListener(lid, pListener);
	}

	// Select the listener type
	ListenerTypeList* pList = SelectListenerList(listenerType);
	if (pList == nullptr)
	{
		return false;
	}

	ListenerID lid = m_listenerID++;
	if (plistenerId)
	{
		*plistenerId = lid;
	}
	return pList->AddListener(contextId, lid, pListener);
}

bool geng::DefaultGame::MoveToFront(ListenerType listenerType, ContextID contextId)
{
	if (listenerType == ListenerType::Executive
	   || contextId == EXECUTIVE_CONTEXT 
	   || contextId >= m_contexts.size())
	{
		return false;
	}

	ListenerTypeList* pList = SelectListenerList(listenerType);
	if (pList == nullptr)
	{
		return false;
	}

	return pList->MoveToFront(contextId);
}

bool geng::DefaultGame::SendToBack(ListenerType listenerType, ContextID contextId)
{
	if (listenerType == ListenerType::Executive
		|| contextId == EXECUTIVE_CONTEXT
		|| contextId >= m_contexts.size())
	{
		return false;
	}

	ListenerTypeList* pList = SelectListenerList(listenerType);
	if (pList == nullptr)
	{
		return false;
	}

	return pList->SendToBack(contextId);
}

bool geng::DefaultGame::MakePredecessor(ListenerType listenerType, ContextID contextId,
	ContextID successorId)
{
	if (listenerType == ListenerType::Executive
		|| contextId == EXECUTIVE_CONTEXT
		|| contextId >= m_contexts.size())
	{
		return false;
	}

	ListenerTypeList* pList = SelectListenerList(listenerType);
	if (pList == nullptr)
	{
		return false;
	}

	return pList->MakePredecessor(contextId, successorId);
}

bool geng::DefaultGame::SetVisibility(ContextID contextId, bool value)
{
	// Change the visibility
	if (contextId == EXECUTIVE_CONTEXT 
		|| contextId >= m_contexts.size())
	{
		return false;
	}


	m_contexts[contextId].m_nextVisible = value;
	if (m_simState.execFrameCount == 0)
	{
		m_contexts[contextId].contextState.visibility.curValue = value;
	}
	return true;
}

bool geng::DefaultGame::SetRunState(ContextID contextId, bool value)
{
	// Change the visibility
	if (contextId == EXECUTIVE_CONTEXT
		|| contextId >= m_contexts.size())
	{
		return false;
	}

	m_contexts[contextId].m_nextRun = value;
	if (m_simState.execFrameCount == 0)
	{
		m_contexts[contextId].contextState.runstate.curValue = value;
	}
	return true;
}

bool geng::DefaultGame::SetFocus(ContextID contextId)
{
	if (contextId == EXECUTIVE_CONTEXT
		|| contextId >= m_contexts.size())
	{
		return false;
	}

	if (m_focus == contextId)
	{
		// nop
		return true;
	}
	// For focus, the logic is different.  Only up to one element can have focus

	if (m_focus != EXECUTIVE_CONTEXT)
	{
		m_contexts[m_focus].m_nextFocus = false;
	}

	if (contextId != EXECUTIVE_CONTEXT)
	{
		if (m_simState.execFrameCount == 0)
		{
			m_contexts[contextId].contextState.focus.curValue = true;
		}
		m_contexts[contextId].m_nextFocus = true;
	}

	m_focus = contextId;

	return true;
}

bool geng::DefaultGame::SetFrameIndex(ContextID contextId, unsigned long frameCount = 0)
{
	if (contextId == EXECUTIVE_CONTEXT
		|| contextId >= m_contexts.size())
	{
		return false;
	}

	m_contexts[contextId].contextState.frameCount = frameCount;
	m_contexts[contextId].contextState.simulatedTime = frameCount * m_gameArgs.msTimePerFrame;
	return true;
}

void geng::DefaultGame::UpdateContextStateBefore()
{
	for (size_t i = 1; i < m_contexts.size(); ++i)
	{
		if (m_contexts[i].contextState.runstate.curValue)
		{
			++m_contexts[i].contextState.frameCount;
			m_contexts[i].contextState.simulatedTime += m_gameArgs.msTimePerFrame;
		}
	}
}

void geng::DefaultGame::ContextInputCallbacks()
{
	auto callInput = [this](ContextID ctxId, const ListenerGroup& lgroup)
	{
		if (m_contexts[ctxId].contextState.focus.curValue
			|| m_contexts[ctxId].contextState.focus.prevValue)
		{
			lgroup.OnFrame(m_simState, &m_contexts[ctxId].contextState);
		}
	};

	m_inputList.IterContexts(callInput);
}

void geng::DefaultGame::ContextSimCallbacks()
{
	auto callSim = [this](ContextID ctxId, const ListenerGroup& lgroup)
	{
		if (m_contexts[ctxId].contextState.runstate.curValue
			|| m_contexts[ctxId].contextState.runstate.prevValue)
		{
			lgroup.OnFrame(m_simState, &m_contexts[ctxId].contextState);
		}
	};

	m_simList.IterContexts(callSim);
}

void geng::DefaultGame::ContextRenderCallbacks()
{
	auto callRender = [this](ContextID ctxId, const ListenerGroup& lgroup)
	{
		if (m_contexts[ctxId].contextState.visibility.curValue
			|| m_contexts[ctxId].contextState.visibility.prevValue)
		{
			lgroup.OnFrame(m_simState, &m_contexts[ctxId].contextState);
		}
	};

	m_renderList.IterContexts(callRender);
}

void geng::DefaultGame::UpdateContextStateAfter()
{
	for (size_t i = 1; i < m_contexts.size(); ++i)
	{
		m_contexts[i].contextState.focus.prevValue = m_contexts[i].contextState.focus.curValue;
		m_contexts[i].contextState.focus.curValue = m_contexts[i].m_nextFocus;

		m_contexts[i].contextState.runstate.prevValue = m_contexts[i].contextState.runstate.curValue;
		m_contexts[i].contextState.runstate.curValue = m_contexts[i].m_nextRun;

		m_contexts[i].contextState.visibility.prevValue = m_contexts[i].contextState.visibility.curValue;
		m_contexts[i].contextState.visibility.curValue = m_contexts[i].m_nextVisible;
	}
}

void geng::DefaultGame::RunGameLoop()
{
	m_lastFrameTime = std::chrono::steady_clock::now();

	std::chrono::milliseconds msForFrame;
	std::chrono::time_point<std::chrono::steady_clock> curFrameTime;
	while (m_isActive)
	{
		// Executive listeners
		m_executiveListeners.OnFrame(m_simState, nullptr);

		UpdateContextStateBefore();
		ContextInputCallbacks();
		ContextSimCallbacks();
		ContextRenderCallbacks();
		UpdateContextStateAfter();

		if (m_isActive)
		{
			++m_simState.execFrameCount;
			m_simState.execSimulatedTime += m_gameArgs.msTimePerFrame;

			curFrameTime = std::chrono::steady_clock::now();

			msForFrame =
				std::chrono::duration_cast<std::chrono::milliseconds>(curFrameTime - m_lastFrameTime);

			// This should be positive
			m_msActualTime += (unsigned long)msForFrame.count();

			if (m_msActualTime - m_msLastSecondTime >= 1000)
			{
				m_actualFPS = m_simState.execFrameCount - m_frameCountAtSecondSwitch;
				m_msLastSecondTime = m_msActualTime;
				m_frameCountAtSecondSwitch = m_simState.execFrameCount;
			}

			// First simulate without IO until m_msSimulatedTime is at least m_msActualTime
			// NOTE:  m_msActualTime *must* be updated - otherwise we are lying!
			// If this loop executes some critical number of times, the simulation is too slow
			// to catch up with realtime at maximum speed (which will happen if many frames take
			// longer to simulate than the number of miliseconds given to a frame)

			m_lastFrameTime = curFrameTime;
			while (m_simState.execSimulatedTime < m_msActualTime)
			{
				m_simState.catchingUp = true;
				m_executiveListeners.OnFrame(m_simState, nullptr);

				ContextInputCallbacks();
				ContextSimCallbacks(); 
				
				// NO rendering

				if (!m_isActive)
				{
					break;
				}

				++m_simState.execFrameCount;
				m_simState.execSimulatedTime += m_gameArgs.msTimePerFrame;

				curFrameTime = std::chrono::steady_clock::now();
				msForFrame =
					std::chrono::duration_cast<std::chrono::milliseconds>(curFrameTime - m_lastFrameTime);

				m_msActualTime += (unsigned long)msForFrame.count();
				m_lastFrameTime = curFrameTime;
			}

			m_simState.catchingUp = false;

			if (!m_isActive)
			{
				break;
			}

			// What happens next depends on the relation between actual and simulated time
			if (m_msActualTime < m_simState.execSimulatedTime)
			{
				// Possibly wait
				if (m_simState.execSimulatedTime - m_msActualTime > m_gameArgs.msBreather)
				{
					unsigned long msSleep = m_simState.execSimulatedTime - m_msActualTime - m_gameArgs.msBreather;
					std::this_thread::sleep_for(std::chrono::milliseconds(msSleep));
				}

				// The breather is silently ignored
				m_msActualTime = m_simState.execSimulatedTime;
			}
		}
	}
}

bool geng::DefaultGame::Run()
{
	// Note: this assumes that the components have been added in least->most dependent order
	auto pThis = shared_from_this();

	for (const auto& pComponent : m_components)
	{
		if (!pComponent->Initialize(pThis))
		{
			return false;
		}
	}

	m_isActive = true;
	RunGameLoop();

	// Wind down in reverse order
	for (auto itRev = m_components.rbegin(); itRev != m_components.rend(); ++itRev)
	{
		(*itRev)->WindDown(pThis);
	}

	return true;
}

void geng::DefaultGame::Quit()
{
	m_isActive = false;
}

void geng::DefaultGame::LogError(const char* pError)
{
	std::cerr << pError << '\n';
}


const std::shared_ptr<geng::IGameComponent>& 
	geng::DefaultGame::GetComponent(const char* pName)
{
	if (m_componentMap.count(pName) == 0)
	{
		static std::shared_ptr<geng::IGameComponent> emptyPtr;
		return emptyPtr;
	}

	auto itComponent = m_componentMap.find(pName);

	return m_components[itComponent->second];
}

const geng::GameArgs& geng::DefaultGame::GetGameArgs() const
{
	return m_gameArgs;
}