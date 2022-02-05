#pragma once

struct RecognitionParams
{
	int graussianBlurSize = 3;
	double graussianSigma = 100.0f;

	double cannyThreshold1 = 25;
	double cannyThreshold2 = 75;

	int dilateStructuringElementSize = 5;

	int thresholdMin = 0;
	int thresholdMax = 255;

	double minCardContourArea = 10000.0;
	double minElementContourArea = 10.0;

	int isolatedCardWith = 320;
	int isolatedCardHeight = 445;

	int rankCutRowsStart = 15;
	int rankCutRowsEnd = 75;
	int rankCutColsStart = 10;
	int rankCutColsEnd = 44;

	int suitCutRowsStart = 70;
	int suitCutRowsEnd = 120;
	int suitCutColsStart = 5;
	int suitCutColsEnd = 50;
};