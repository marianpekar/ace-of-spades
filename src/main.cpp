#pragma once

#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>

#include "CardsRecognizer.h"
#include "InputController.h"

int main()
{
	using Framerate = std::chrono::duration<std::chrono::steady_clock::rep, std::ratio<1, 60>>;

	CardsRecognizerCreateInfo createInfo;
	RecognitionParams recognitionParams;
	CardsRecognizer cardsRecognizer(createInfo, recognitionParams);

	InputController inputController; 

	std::vector<CardCode> recognizedCards;

	std::mutex mtx;
	auto processCards = [&inputController, &recognizedCards, &mtx]()
	{
		int keyDuration = 200;
		int moveKeyDelay = 50;
		int actionKeyDelay = 800;

		auto next = std::chrono::steady_clock::now() + Framerate{ 1 };	
		while (true) {
			mtx.lock();
			for (auto& cardCode : recognizedCards)
			{
				switch (cardCode)
				{
				case CardCode::EIGHT_OF_DIAMONDS:
					inputController.pushKey(VK_NUMPAD8, keyDuration, moveKeyDelay);
					break;

				case CardCode::TWO_OF_DIAMONDS:
					inputController.pushKey(VK_NUMPAD2, keyDuration, moveKeyDelay);
					break;

				case CardCode::FOUR_OF_DIAMONDS:
					inputController.pushKey(VK_NUMPAD4, keyDuration, moveKeyDelay);
					break;

				case CardCode::SIX_OF_DIAMONDS:
					inputController.pushKey(VK_NUMPAD6, keyDuration, moveKeyDelay);
					break;

				case CardCode::ACE_OF_HEARTS:
					inputController.pushKey(VK_SPACE, keyDuration, actionKeyDelay);
					break;

				case CardCode::ACE_OF_SPADES:
					inputController.pushKey(VK_CONTROL, keyDuration, actionKeyDelay);
					break;
				default:
					break;
				}
			}
			inputController.processInputs();
			mtx.unlock();

			std::this_thread::sleep_until(next);
			next += Framerate{ 1 };
		}
	};

	std::thread t(processCards);

	auto next = std::chrono::steady_clock::now() + Framerate{ 1 };
	while (true)
	{	
		cardsRecognizer.processVideoInput();
		recognizedCards = cardsRecognizer.getRecognizedCards();

		std::this_thread::sleep_until(next);
		next += Framerate{ 1 };
	}

	return 0;
}