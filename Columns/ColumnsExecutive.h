#pragma once

#include "BaseGameComponent.h"
#include "ActionMapper.h"
#include "SDLInput.h"
#include "ColumnsSim.h"
#include <memory>

namespace geng::columns
{
	enum class ContextDesc
	{
		ActiveGame,
		PausedGame,
		NoGame
	};

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
		static const char* GetColumnsInputBridgeName();
		static const char* GetExecutiveName();

		// Create all the other components and add them to the game
		ColumnsExecutive();
		bool AddToGame(const std::shared_ptr<IGame>& pGame);

		void OnFrame(const SimState& rSimState,
			const SimContextState* pContextState) override;

		bool IsInitialized() const { return m_initialized; }
		bool IsPaused() const { return m_contextDesc == ContextDesc::PausedGame; }
		bool IsInGame() const { return m_contextDesc != ContextDesc::NoGame; }
	private:
		static void MapActions(ActionMapper& rMapper);

		static bool IsKeyPressedOnce(const KeyState& keyState)
		{
			return (keyState.finalState == KeySignal::KeyDown && keyState.numChanges > 0)
				|| (keyState.finalState == KeySignal::KeyUp && keyState.numChanges > 1);
		}

		bool m_initialized;

		std::shared_ptr<IGame> m_pGame;
		std::shared_ptr<sdl::Input>  m_pInput;
		std::shared_ptr<ColumnsSim> m_pSim;
		ContextID m_simContextId;
		ContextDesc m_contextDesc{ ContextDesc::NoGame };
	};

}