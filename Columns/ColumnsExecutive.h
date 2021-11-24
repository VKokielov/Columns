#pragma once

#include "BaseGameComponent.h"
#include <memory>

namespace geng::columns
{


	class ColumnsExecutive : public BaseGameComponent,
		public IGameListener,
		public std::enable_shared_from_this<ColumnsExecutive>
	{
	public:
		// Data
		static const char* GetDropActionName();
		static const char* GetShiftLeftActionName();
		static const char* GetShiftRightActionName();
		static const char* GetRotateActionName();
		static const char* GetPermuteActionName();
		static const char* GetColumnsSimContextName();
		static const char* GetActionMapperName();


		// Create all the other components and add them to the game
		ColumnsExecutive(const std::shared_ptr<IGame>& pGame);
		void OnFrame(const SimState& rSimState,
			const SimContextState* pContextState) override;

	private:
		bool m_initialized;
	};

}