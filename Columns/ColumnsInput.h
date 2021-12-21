#pragma once

#include "BaseGameComponent.h"
#include "ActionCommands.h"
#include "ActionMapper.h"
#include "ActionTranslator.h"
#include "CommandManager.h"
#include "DataPacket.h"
#include "ColumnsData.h"

#include <vector>
#include <memory>
#include <random>

namespace geng::columns
{
	class ColumnsExecutive;

	struct ActionDesc
	{
		const char* pName;
		unsigned int throttlePeriod;

		ActionDesc(const char* pName_, unsigned int throttlePeriod_)
			:pName(pName_),
			throttlePeriod(throttlePeriod_)
		{ }
	};


	class ColumnsInput : public IGameListener,
		public BaseGameComponent
	{
	private:
		struct ActionCommand_
		{
			std::string actionName;
			std::string actionKey;
			unsigned int playerId;
			std::shared_ptr<ActionCommand>  pCommand;
			ActionID actionId{ 0 };
			unsigned int throttlePeriod; // only for player commands

			ActionCommand_(const std::string& actionName_,
				const std::string& actionKey_, 
				unsigned int playerId_,
				const std::shared_ptr<ActionCommand>& pCommand_,
				unsigned int throttlePeriod_)
				:actionName(actionName_),
				actionKey(actionKey_),
				playerId(playerId_),
				pCommand(pCommand_),
				throttlePeriod(throttlePeriod_)
			{
				
			}
		};

	public:
		ColumnsInput(const std::vector<ActionDesc>& vActions, unsigned int playerCount,
					 unsigned long msPerFrame);

		bool Initialize(const std::shared_ptr<IGame>& pGame);

		void OnStartGame(const ColumnsArgs& args);
		void OnFrame(const SimState& rSimState,
			const SimContextState* pContextState) override;
		void OnPauseGame(bool pauseState) { }
		void OnEndGame();

		ActionCommandID GetIDFor(const char* pActionName,
			unsigned int playerId,
			bool* pFound = nullptr) const;

		bool GetActionState(ActionCommandID cmdId) const
		{
			return cmdId < m_actionCommands.size() ?
				m_actionCommands[cmdId].pCommand->GetState()
				: false;
		}

		unsigned long GetRandomNumber(unsigned long min, unsigned long upperBound);

		const SimArgs* GetSimArgs() const
		{
			return &(m_pSimArgsPacket->Get());
		}

		bool IsPlaybackVersion(uint32_t formatVersion) const
		{
			return m_pCommandManager
				&& m_pCommandManager->GetPBMode() == PlaybackMode::Playback
				&& m_pCommandManager->GetFormatVersion() == formatVersion;
		}

		bool GetPlaybackVersion(uint32_t& formatVersion) const
		{
			if (m_pCommandManager && m_pCommandManager->GetPBMode() == PlaybackMode::Playback)
			{
				formatVersion = m_pCommandManager->GetFormatVersion();
				return true;
			}

			return false;
		}

	private:
		// Actions

		// data
		std::vector<std::string>  m_actionNames;
		std::vector<ActionCommand_> m_actionCommands;
		unsigned long m_msPerFrame;

		// objects
		std::shared_ptr<serial::DataPacket<SimArgs> > m_pSimArgsPacket;
		std::shared_ptr<ActionMapper>  m_actionMapper;
		std::shared_ptr<ActionTranslator> m_actionTranslator;
		std::shared_ptr<IInput>  m_pInput;
		std::weak_ptr<ColumnsExecutive>  m_pExecutive;

		std::mt19937_64  m_generator;

		// Command manager
		std::shared_ptr<CommandManager>   m_pCommandManager;

		bool m_valid{ false };
		std::string m_error;
	};



}