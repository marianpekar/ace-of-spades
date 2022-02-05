#pragma once

#include <windows.h>
#include <thread>
#include <iostream>
#include <queue>
#include <vector>

class InputController
{
private:
	INPUT input;

	struct InputData {
		WORD vkKey;
		int durationMs;
		int delayMs;
	};

	std::queue<InputData> inputs;
	std::vector<WORD> pressedKeys;

	void holdKey(WORD vkKey, int durationMs, int delayMs);
public:
	InputController();

	void pushKey(WORD vkKey, int durationMs, int delayMs);
	void processInputs();
};