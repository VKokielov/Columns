#include "ActionMapper.h"
#include <algorithm>

geng::ActionMapper::ActionMapper(const char* pName)
	:BaseGameComponent(pName)
{

}

geng::ActionID geng::ActionMapper::CreateAction(const char* pName)
{
	std::string actionName(pName);

	auto itAction = m_actionNameMap.find(actionName);

	ActionID id{ 0 };
	if (itAction == m_actionNameMap.end())
	{
		id = (ActionID)m_actions.size();
		m_actions.emplace_back(id,actionName);
		m_actionNameMap.emplace(actionName, id);
	}
	else
	{
		id = (ActionID)itAction->second;
	}

	return id;
}

geng::ActionID geng::ActionMapper::GetAction(const char* pName) const
{
	std::string actionName(pName);

	auto itAction = m_actionNameMap.find(actionName);

	ActionID id{ 0 };
	if (itAction != m_actionNameMap.end())
	{
		return (ActionID)itAction->second;
	}

	return INVALID_ACTION;
}

bool geng::ActionMapper::ClearMapping(ActionID actionId)
{
	if (actionId < 0 || actionId >= m_actions.size())
	{
		return false;
	}

	m_actions[actionId].actionMapping.keyGroups.clear();
	for (const auto& pListener : m_vMappingListeners)
	{
		pListener->OnMapping(actionId, m_actions[actionId].actionMapping);
	}

	return true;
}

void geng::ActionMapper::GetAllMappings(const std::shared_ptr<IActionMappingListener>& pListener)
{
	
	for (ActionID id = 0; id < m_actions.size(); ++id)
	{
		pListener->OnMapping(id, m_actions[id].actionMapping);
	}
}
void geng::ActionMapper::AddMappingListener(const std::shared_ptr<IActionMappingListener>& pListener)
{
	if (std::find(m_vMappingListeners.begin(), m_vMappingListeners.end(), pListener)
		!= m_vMappingListeners.end())
	{
		return;
	}

	m_vMappingListeners.emplace_back(pListener);
}
