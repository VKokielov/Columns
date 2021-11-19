#include "DefaultGame.h"

#include <iostream>


geng::DefaultGame::DefaultGame(const GameArgs& args)
	:m_gameArgs(args)
{
	SimState simState;
	simState.timePerFrame = args.timePerFrame;
	simState.quality = SimQuality::Running;

	m_frameManager.UpdateSimState(simState, FID_QUALITY | FID_TPF);
}

std::shared_ptr<geng::DefaultGame> geng::DefaultGame::CreateGame(const GameArgs& args)
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