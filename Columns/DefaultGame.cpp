#include "DefaultGame.h"

#include <iostream>

bool geng::DefaultGame::AddComponent(const char* pName, const std::shared_ptr<IGameComponent>& pComponent)
{
	// Try to add
	std::string sname{ pName };
	if (m_componentMap.count(sname) != 0)
	{
		return false;
	}

	m_componentMap.emplace(sname, m_components.size());
	m_components.emplace_back(pComponent);

	auto componentType = pComponent->GetType();
	if (componentType == GameComponentType::IO
		|| componentType == GameComponentType::Simulation)
	{
		// This should succeed if the class implements IFrameListener!
		std::shared_ptr<IFrameListener> pFL = std::dynamic_pointer_cast<IFrameListener>(pComponent);
		if (!pFL)
		{
			return false;
		}

		m_frameManager.Subscribe(pFL, componentType == GameComponentType::Simulation);
	}

	return true;
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

	m_frameManager.Simulate();

	// Wind down in reverse order
	for (auto itRev = m_components.rbegin(); itRev != m_components.rend(); ++itRev)
	{
		(*itRev)->WindDown(pThis);
	}

	return true;
}

void geng::DefaultGame::LogError(const char* pError)
{
	std::cerr << pError << '\n';
}