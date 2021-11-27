#pragma once

#include "IInput.h"

#include <vector>
#include <optional>

namespace geng::sdl
{
	void AddAllTextKeys(std::vector<KeyCode>& kcodes);
	char GetTextFromKeyCode(KeyCode kcode);

}