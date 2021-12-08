#pragma once

#include "BaseGameComponent.h"
#include "ActionMapper.h"
#include "SDLInput.h"
#include "ColumnsSim.h"
#include "CheatTrie.h"
#include <memory>

namespace geng::columns
{
	enum class ContextDesc
	{
		ActiveGame,
		PausedGame,
		NoGame
	};

	struct ThrottleSettings
	{
		unsigned int dropThrottlePeriod;
		unsigned int nonDropThrottlePeriod;
	};

	class ColumnsExecutive : public BaseGameComponent,
		public IGameListener,
		public std::enable_shared_from_this<ColumnsExecutive>
	{
	public:
		// Data
		static constexpr unsigned int msToEnterCheat = 700;

		// These functions are expected to return pointers to statically allocated
		// constant strings, so they can be set inside structs without constructing std::strings
		// around them
		static const char* GetDropActionName();
		static const char* GetShiftLeftActionName();
		static const char* GetShiftRightActionName();
		static const char* GetRotateActionName();
		static const char* GetPermuteActionName();
		static const char* GetColumnsSimContextName();
		static const char* GetActionMapperName();
		static const char* GetColumnsInputBridgeName();
		static const char* GetColumnsInputComponentName();
		static const char* GetExecutiveName();

		// Create all the other components and add them to the game
		ColumnsExecutive();
		bool AddToGame(const std::shared_ptr<IGame>& pGame);

		void OnFrame(const SimState& rSimState,
			const SimContextState* pContextState) override;

		bool IsInitialized() const { return m_initialized; }
		bool IsPaused() const { return m_contextDesc == ContextDesc::PausedGame; }
		bool IsInGame() const { return m_contextDesc != ContextDesc::NoGame; }

		// Only valid when the game is active

		void UpdateCheatState(unsigned long execTime);
		void AddCheat(const char* pText, CheatKey key);
		bool HasCheat() const
		{
			return m_cheatKey.has_value();
		}
		CheatKey GetCheat() const
		{
			return m_cheatKey.has_value() ? *m_cheatKey : CheatKey();
		}
		void ResetCheat()
		{
			m_cheatKey.reset();
			m_cheatState = CHEAT_TRIE_ROOT;
		}
	private:
		static void MapActions(ActionMapper& rMapper,
							std::vector<ActionDesc>& columnsActions,
							const ThrottleSettings& throttleSettings);

		static bool IsKeyPressedOnce(const KeyState& keyState)
		{
			return (keyState.finalState == KeySignal::KeyDown && keyState.numChanges > 0)
				|| (keyState.finalState == KeySignal::KeyUp && keyState.numChanges > 1);
		}

		void SetupCheats(IInput* pInput);

		bool m_initialized;

		std::shared_ptr<IGame> m_pGame;
		std::shared_ptr<sdl::Input>  m_pInput;
		std::shared_ptr<ColumnsInput>  m_pColumnsInput;
		std::shared_ptr<ColumnsSim> m_pSim;
		geng::columns::ColumnsSimArgs m_simArgs;
		ContextID m_simContextId;
		ContextDesc m_contextDesc{ ContextDesc::NoGame };

		
		bool m_cheatsEnabled{ true };
		std::vector<KeyState> m_alphaKeyStates;
		std::vector<KeyState*> m_alphaKeyRefs;
 
		// Cheat state
		CheatTrie m_cheatTrie;
		TrieIndex m_cheatState{ CHEAT_TRIE_ROOT };
		unsigned long m_lastCheatInputTime;
		std::optional<CheatKey> m_cheatKey;
	};

	// Helper function for getting cheats
	inline bool FetchCheat(CheatKey cheatKey, ColumnsExecutive& columnsExec)
	{
		if (!columnsExec.HasCheat() || columnsExec.GetCheat() != cheatKey)
		{
			return false;
		}

		columnsExec.ResetCheat();

		return true;
	}

}