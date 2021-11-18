// Columns.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include <unordered_set>
#include <unordered_map>
#include <set>
#include <map>
#include <utility>
#include <algorithm>
#include <string>

#include "SimStateDispatcher.h"

using namespace std;

struct BlueState { };

struct RedState { };

class StateHolder : public geng::SimStateDispatcher<StateHolder,
													RedState, 
													RedState, BlueState>
{
public:
	std::string label;
};

class StateProcessor
{
public:
	void OnState(const BlueState&, 
		StateHolder& h,
		int x)
	{

		std::cout << "In blue state; h " << h.label << " x " << x << '\n';

		h.Transition<RedState>();
	}

	void OnState(const RedState&,
		StateHolder& h,
		int x)
	{
		std::cout << "In red state; h " << h.label << " x " << x << '\n';
		h.Transition<BlueState>();
	}
};


int main()
{
	StateHolder sHolder;
	sHolder.label = "my_state_holder";

	StateProcessor sProcessor;

	sHolder.Dispatch(sProcessor, sHolder, 1);
	sHolder.Dispatch(sProcessor, sHolder, 2);
	sHolder.Dispatch(sProcessor, sHolder, 3);
	sHolder.Dispatch(sProcessor, sHolder, 4);

}
