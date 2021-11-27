#include "SDLTextKeycodes.h"

#include <SDL.h>
#include <array>
#include <algorithm>
#include <iterator>
#include <unordered_map>

void geng::sdl::AddAllTextKeys(std::vector<KeyCode>& kcodesOutput)
{
	static std::array kcodes
		= {SDLK_a,SDLK_b,SDLK_c,SDLK_d,SDLK_e,SDLK_f,SDLK_g,SDLK_h,
		   SDLK_i,SDLK_j,SDLK_k,SDLK_l,SDLK_m,SDLK_n,SDLK_o,SDLK_p,
		   SDLK_q,SDLK_r,SDLK_s,SDLK_t,SDLK_u,SDLK_v,SDLK_w,SDLK_x,
		   SDLK_y, SDLK_z, SDLK_0, SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5,
		   SDLK_6, SDLK_7, SDLK_8, SDLK_9};

	std::copy(kcodes.begin(), kcodes.end(), std::back_inserter(kcodesOutput));
}

char geng::sdl::GetTextFromKeyCode(KeyCode kcode)
{
	static std::unordered_map<KeyCode,char>
		kcodeMap
	{
		std::make_pair(SDLK_a, 'a'),
		std::make_pair(SDLK_b, 'b'),
		std::make_pair(SDLK_c, 'c'),
		std::make_pair(SDLK_d, 'd'),
		std::make_pair(SDLK_e, 'e'),
		std::make_pair(SDLK_f, 'f'),
		std::make_pair(SDLK_g, 'g'),
		std::make_pair(SDLK_h, 'h'),
		std::make_pair(SDLK_i, 'i'),
		std::make_pair(SDLK_j, 'j'),
		std::make_pair(SDLK_k, 'k'),
		std::make_pair(SDLK_l, 'l'),
		std::make_pair(SDLK_m, 'm'),
		std::make_pair(SDLK_n, 'n'),
		std::make_pair(SDLK_o, 'o'),
		std::make_pair(SDLK_p, 'p'),
		std::make_pair(SDLK_q, 'q'),
		std::make_pair(SDLK_r, 'r'),
		std::make_pair(SDLK_s, 's'),
		std::make_pair(SDLK_t, 't'),
		std::make_pair(SDLK_u, 'u'),
		std::make_pair(SDLK_v, 'v'),
		std::make_pair(SDLK_w, 'w'),
		std::make_pair(SDLK_x, 'x'),
		std::make_pair(SDLK_y, 'y'),
		std::make_pair(SDLK_z, 'z'),
		std::make_pair(SDLK_0, '1'),
		std::make_pair(SDLK_1, '1'),
		std::make_pair(SDLK_2, '2'),
		std::make_pair(SDLK_3, '3'),
		std::make_pair(SDLK_4, '4'),
		std::make_pair(SDLK_5, '5'),
		std::make_pair(SDLK_6, '6'),
		std::make_pair(SDLK_7, '7'),
		std::make_pair(SDLK_8, '8'),
		std::make_pair(SDLK_9, '9')
	};

	auto itChar = kcodeMap.find(kcode);
	if (itChar != kcodeMap.end())
	{
		return itChar->second;
	}

	return '\0';
}