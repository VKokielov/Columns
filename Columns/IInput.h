#pragma once


#include "IGame.h"
#include <cinttypes>

namespace geng
{
	using KeyCode = unsigned int;

	enum class KeySignal
	{
		KeyUp,   // 0
		KeyDown // 2
	};
	// Keyboard and mouse button input
	struct KeyState
	{
		KeyCode keyCode;  // [in]
		unsigned int numChanges;  // [out]
		KeySignal finalState;  // [out]
	};

	struct KeyboardState
	{
		// Number of keys that were down at any moment in the last frame
		unsigned int numKeysDownInFrame{ 0 };
	};

	struct MouseState
	{
		unsigned int x{};
		unsigned int y{};
	};

	class IInput : public IGameComponent
	{
	public:
		virtual ~IInput() = default;

		virtual void AddCode(KeyCode code) = 0;

		virtual bool QueryInput(MouseState* pMouseState,
			KeyboardState* pkeyboardState,
			KeyState** ppKeyStates,
			size_t nKeyStates) = 0;
	};


}