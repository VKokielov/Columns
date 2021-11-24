#pragma once
#include "IInput.h"
#include "BaseGameComponent.h"

#include <unordered_map>

namespace geng
{
	class InputBridge : public TemplatedGameComponent<IInput>,
		public IGameListener
	{
	private:
		struct KeyData_
		{
			KeyState state;
		};
	public:
		InputBridge(const char* pName, const std::shared_ptr<IInput>& pUnderlying);

		void AddCode(KeyCode code) override;
		void OnFrame(const SimState& simState, const SimContextState* pContextState) override;
		bool QueryInput(MouseState* pMouseState,
			KeyboardState* pkeyboardState,
			KeyState** ppKeyStates,
			size_t nKeyStates) override;

	private:
		MouseState m_mouseState;
		KeyboardState m_keyboardState;
		std::shared_ptr<IInput> m_pUnderlying;
		std::unordered_map<KeyCode, KeyData_>   m_keyState;
		std::vector<KeyState*> m_stateRefs;
		unsigned int m_downKeys{ 0 };
	};
}