#pragma once

#include "IGame.h"

#include "DefaultFrameManager.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

namespace geng
{

	class DefaultGame : public IGame, public std::enable_shared_from_this<DefaultGame>
	{
	public:
		bool AddComponent(const char* pName, const std::shared_ptr<IGameComponent>& pComponent);

		bool Run() override;

		void LogError(const char* pError) override;
	private:
		std::unordered_map<std::string, size_t>
			m_componentMap;

		std::vector<std::shared_ptr<IGameComponent> > m_components;

		DefaultFrameManager  m_frameManager;
	};

}