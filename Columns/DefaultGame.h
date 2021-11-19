#pragma once

#include "IGame.h"

#include "DefaultFrameManager.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

namespace geng
{
	struct GameArgs
	{
		FrameMgrArgs frameArgs;
	};

	class DefaultGame : public IGame, 
		public std::enable_shared_from_this<DefaultGame>
	{
	public:
		static std::shared_ptr<DefaultGame> CreateGame(const GameArgs& args);

		bool AddComponent(const std::shared_ptr<IGameComponent>& pComponent);

		const std::shared_ptr<IGameComponent>& GetComponent(const char* pName) override;

		bool Run() override;

		void LogError(const char* pError) override;
	private:
		DefaultGame(const GameArgs& args);

		std::unordered_map<std::string, size_t>
			m_componentMap;

		std::vector<std::shared_ptr<IGameComponent> > m_components;

		DefaultFrameManager  m_frameManager;

		GameArgs m_gameArgs;
	};

}