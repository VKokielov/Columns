#pragma once

#include "BaseGameComponent.h"
#include <cinttypes>

namespace geng::sdl
{
	struct SetupArgs
	{
		uint32_t initFlags{0};
	};

	class SetUp : public BaseGameComponent
	{
	public:
		SetUp(const SetupArgs& args)
			:BaseGameComponent("SDLSetUp", GameComponentType::SetUp),
			m_args(args)
		{ }

		bool Initialize(const std::shared_ptr<IGame>& pGame) override;
		void WindDown(const std::shared_ptr<IGame>& pGame) override;

	private:
		SetupArgs m_args;
	};


}