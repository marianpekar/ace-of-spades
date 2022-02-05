#pragma once

#include "CardCodes.h"

struct CardsRecognizerCreateInfo 
{
	int captureDeviceIndex = 0;
	int captureFrameWidth = 800;
	int captureFrameHeight = 600;

	std::string ranksReferenceImageAssetsPath = "assets/ranks";

	std::vector<std::string> ranksReferenceImageFilenames = { 
		"two", 
		"three", 
		"four", 
		"five", 
		"six", 
		"seven", 
		"eight", 
		"nine", 
		"ten", 
		"jack", 
		"queen", 
		"king", 
		"ace" 
	};

	std::vector<CardCode> ranksCardCodes = { 
		CardCode::TWO, 
		CardCode::THREE, 
		CardCode::FOUR, 
		CardCode::FIVE, 
		CardCode::SIX, 
		CardCode::SEVEN, 
		CardCode::EIGHT, 
		CardCode::NINE,
		CardCode::TEN,
		CardCode::JACK, 
		CardCode::QUEEN, 
		CardCode::KING, 
		CardCode::ACE 
	};

	std::string suitReferenceImageAssetsPath = "assets/suits";

	std::vector<std::string> suitsReferenceImageFilenames = { 
		"clubs", 
		"diamonds", 
		"hearts", 
		"spades" 
	};

	std::vector<CardCode> suitsCardCodes = {
		CardCode::CLUBS,
		CardCode::DIAMONDS,
		CardCode::HEARTS,
		CardCode::SPADES
	};

	std::string referenceImagesFileExtension = ".png";
};