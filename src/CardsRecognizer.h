#pragma once

#include <vector>
#include <string>
#include <queue>

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include "CardsRecognizerCreateInfo.h"
#include "RecognitionParams.h"
#include "CardCodes.h"

class CardsRecognizer 
{
private:
	struct ReferenceSet
	{
		std::vector<std::string> filenames;
		std::vector<CardCode> imageCodes;
		std::vector<cv::Mat> images;
		int imageWidth = -1;
		int imageHeight = -1;
		
	};

	ReferenceSet ranks, suits;

	void loadReferenceImages(ReferenceSet& referenceSet, const std::string& path, const std::vector<std::string>& filenames, const std::string& extension);

	RecognitionParams recognitionParams;

	void preProcessForContoursFinding(cv::Mat& image) const;
	void preProcessForRecognition(cv::Mat& image) const;

	void isolateCards(cv::Mat& image, const cv::Mat& originalImage, float w, float h, std::queue<cv::Mat>& cards);
	cv::Mat isolateElement(cv::Mat& image) const;
	CardCode findMatch(cv::Mat& image, const ReferenceSet referenceSet, const std::string& debugWindowTitle);

	cv::VideoCapture cap;
	cv::Mat frame, originalFrame;

	std::queue<cv::Mat> cards;
	std::vector<CardCode> recognizedCards;

public:
	CardsRecognizer(CardsRecognizerCreateInfo createInfo, RecognitionParams recognitionParams);
	~CardsRecognizer();

	void processVideoInput();

	std::vector<CardCode> getRecognizedCards() const;
};