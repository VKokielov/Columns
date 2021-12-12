#pragma once

#include "BaseGameComponent.h"
#include "ActionMapper.h"
#include "SDLInput.h"
#include "ColumnsSim.h"
#include "ColumnsInput.h"
#include "CheatTrie.h"
#include "SimStateDispatcher.h"
#include <memory>

namespace geng::columns
{
	enum class ContextState
	{
		NoGame,
		ActiveGame,
		PausedGame	
	};

	struct ThrottleSettings
	{
		unsigned int dropThrottlePeriod;
		unsigned int nonDropThrottlePeriod;
	};

	struct NoGameState { };

	struct ActiveGameState { };

	struct PausedGameState { };

	class ColumnsExecutive : public BaseGameComponent,
		public IGameListener,
		public SimStateDispatcher<ColumnsExecutive, NoGameState, NoGameState, ActiveGameState, PausedGameState>,
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

		// State changes
		template<typename ... Args>
		struct OnFrameSelector
		{
			static auto Select() 
				{ return static_cast<void (ColumnsExecutive::*)(Args...)>(&ColumnsExecutive::OnFrame); }
		};

		ColumnsExecutive();
		bool AddToGame(const std::shared_ptr<IGame>& pGame);

		void StartGame();
		void EndGame();
		void PauseGame(bool pauseState);

		void OnEnterState(NoGameState& ngs);
		void OnExitState(NoGameState& ngs);
		void OnEnterState(ActiveGameState& ags);
		void OnExitState(ActiveGameState& ags);
		void OnEnterState(PausedGameState& ags);
		void OnExitState(PausedGameState& ags);
		
		void OnFrame(NoGameState&, const SimState& rSimState);
		void OnFrame(ActiveGameState&, const SimState& rState);
		void OnFrame(PausedGameState&, const SimState& rState);

		void OnFrame(const SimState& rSimState,
			const SimContextState* pContextState) override;

		bool IsInitialized() const { return m_initialized; }
		static bool IsPausedState(ContextState state) 
		{ 
			return state == ContextState::PausedGame; 
		}

		bool IsPaused() const { return IsPausedState(m_contextState); }

		static bool IsInGameState(ContextState state)
		{ 
			return state != ContextState::NoGame; 
		}

		bool IsInGame() const { return IsInGameState(m_contextState); }

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

		// Add a key state for a per-frame subscription
		void AddKeySub(KeyState* pkeyState);
		void ResetKey(KeyState* pKeyState);

		void SetupCheats(IInput* pInput);

		bool m_initialized{ false };

		std::shared_ptr<IGame> m_pGame;
		std::shared_ptr<sdl::Input>  m_pInput;
		std::shared_ptr<ColumnsInput>  m_pColumnsInput;
		std::shared_ptr<ColumnsSim> m_pSim;
		
		geng::columns::InputArgs m_inputArgs;
		geng::columns::ColumnsSimArgs m_simArgs;

		ContextID m_simContextId;
		ContextState m_contextState{ ContextState::NoGame };
		ContextState m_prevContextState{ ContextState::NoGame };

		// Keyboard states
		std::vector<KeyState*> m_keySubs;

		bool m_cheatsEnabled{ true };
		std::vector<KeyState> m_alphaKeyStates;
		std::vector<KeyState*> m_alphaKeyRefs;
 
		// Cheat state
		CheatTrie m_cheatTrie;
		TrieIndex m_cheatState{ CHEAT_TRIE_ROOT };
		unsigned long m_lastCheatInputTime;
		std::optional<CheatKey> m_cheatKey;

		// Keys: start, pause, escape
		// NOTE:  The "escape" key will someday designate "open the menu"
		KeyState m_spaceKey;
		KeyState m_pauseKey;
		KeyState m_escKey;
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