#include "InputController.h"

InputController::InputController()
{
	input.type = INPUT_KEYBOARD;
	input.ki.time = 0;
	input.ki.dwExtraInfo = 0;
}

void InputController::pushKey(WORD vkKey, int durationMs, int delayMs)
{
	if (std::count(pressedKeys.begin(), pressedKeys.end(), vkKey))
		return;

	inputs.push({vkKey, durationMs, delayMs});
}

void InputController::processInputs()
{
	while (inputs.size() > 0)
	{
		InputData inputData = inputs.front();
		inputs.pop();

		auto f = [this](WORD vkKey, int durationMs, int delayMs) { this->holdKey(vkKey, durationMs, delayMs); };
		std::thread t(f, inputData.vkKey, inputData.durationMs, inputData.delayMs);
		t.detach();
	}
}

void InputController::holdKey(WORD vkKey, int durationMs, int delayMs)
{
	input.ki.wScan = MapVirtualKey(vkKey, MAPVK_VK_TO_VSC);
	input.ki.wVk = vkKey;

	input.ki.dwFlags = 0;
	SendInput(1, &input, sizeof(INPUT));
	pressedKeys.push_back(vkKey);

#if _DEBUG 
	std::cout << vkKey << " Pressed" << std::endl;
#endif

	std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));

	input.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &input, sizeof(INPUT));

#if _DEBUG 
	std::cout << vkKey << " Released" << std::endl;
#endif

#if _DEBUG 
	std::cout << vkKey << " Locked" << std::endl;
#endif
	std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
	for (size_t i = 0; i < pressedKeys.size(); i++)
	{
		if (pressedKeys[i] == vkKey)
		{
			pressedKeys.erase(pressedKeys.begin() + i);
#if _DEBUG 
			std::cout << vkKey << " Unlocked" << std::endl;
#endif
		}
	}
}