#pragma once


#include "IGame.h"
#include <cinttypes>

namespace geng
{
	using KeyCode = unsigned int;

	enum class KeySignal
	{
		KeyUp,
		KeyPressed,
		KeyDown,
		KeyReleased
	};
	// Keyboard and mouse button input
	struct KeyState
	{
		KeyCode keyCode;  // [in]
		KeySignal signal;  // [out]
	};

	struct MouseState
	{
		unsigned int x;
		unsigned int y;
	};

	class IInput : public IGameComponent
	{
	public:
		virtual ~IInput() = default;

		virtual void AddCode(KeyCode code) = 0;

		virtual bool QueryInput(MouseState* pMouseState,
			KeyState** ppKeyStates,
			size_t nKeyStates) = 0;

		virtual unsigned long GetRandomNumber(unsigned long min, unsigned long upperBound) = 0;
	};


}