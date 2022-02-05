#include "CardsRecognizer.h"

CardsRecognizer::CardsRecognizer(CardsRecognizerCreateInfo createInfo, RecognitionParams recognitionParams)
{
	cap = cv::VideoCapture(createInfo.captureDeviceIndex);

	cap.set(cv::CAP_PROP_FRAME_WIDTH, createInfo.captureFrameWidth);
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, createInfo.captureFrameHeight);

	ranks.filenames = createInfo.ranksReferenceImageFilenames;
	ranks.imageCodes = createInfo.ranksCardCodes;
	loadReferenceImages(ranks, createInfo.ranksReferenceImageAssetsPath, createInfo.ranksReferenceImageFilenames, createInfo.referenceImagesFileExtension);

	suits.filenames = createInfo.suitsReferenceImageFilenames;
	suits.imageCodes = createInfo.suitsCardCodes;
	loadReferenceImages(suits, createInfo.suitReferenceImageAssetsPath, createInfo.suitsReferenceImageFilenames, createInfo.referenceImagesFileExtension);

	this->recognitionParams = recognitionParams;
}


void CardsRecognizer::loadReferenceImages(ReferenceSet& referenceSet, const std::string& path, const std::vector<std::string>& filenames, const std::string& extension)
{
	std::vector<cv::Mat> images;
	for (auto& filename : filenames)
	{
		cv::Mat image = cv::imread(path + "/" + filename + extension);
		cv::cvtColor(image, image, cv::COLOR_BGR2GRAY, 1);
		images.push_back(image);

		if (referenceSet.imageWidth == -1)
		{
			referenceSet.imageWidth = image.cols;
		}

		if (referenceSet.imageHeight == -1)
		{
			referenceSet.imageHeight = image.rows;
		}
	}

	referenceSet.images = images;
}

void CardsRecognizer::preProcessForContoursFinding(cv::Mat& image) const
{
	cv::cvtColor(image, image, cv::COLOR_BGR2GRAY, 1);
	int n = recognitionParams.graussianBlurSize;
	cv::GaussianBlur(image, image, cv::Size(n, n), recognitionParams.graussianSigma);

	cv::Canny(image, image, recognitionParams.cannyThreshold1, recognitionParams.cannyThreshold2);

	int size = recognitionParams.dilateStructuringElementSize;
	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(size, size));
	cv::dilate(image, image, element);
}

void CardsRecognizer::preProcessForRecognition(cv::Mat& image) const
{
	cv::cvtColor(image, image, cv::COLOR_BGR2GRAY, 1);

	int thresholdType = cv::THRESH_BINARY | cv::THRESH_OTSU;
	cv::threshold(image, image, recognitionParams.thresholdMin, recognitionParams.thresholdMax, thresholdType);
}

void CardsRecognizer::isolateCards(cv::Mat& image, const cv::Mat& originalImage, float w, float h, std::queue<cv::Mat>& cards)
{
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;

	cv::findContours(image, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	std::vector<std::vector<cv::Point>> contoursPoly(contours.size());

	for (size_t i = 0; i < contours.size(); i++)
	{
		double area = cv::contourArea(contours[i]);
		double peri = cv::arcLength(contours[i], true);
		cv::approxPolyDP(contours[i], contoursPoly[i], 0.05f * peri, true);

		if (contoursPoly[i].size() == 4 && area > recognitionParams.minCardContourArea)
		{
			cv::Point2f tl = { (float)contoursPoly[i][0].x, (float)contoursPoly[i][0].y };
			cv::Point2f bl = { (float)contoursPoly[i][1].x, (float)contoursPoly[i][1].y };
			cv::Point2f br = { (float)contoursPoly[i][2].x, (float)contoursPoly[i][2].y };
			cv::Point2f tr = { (float)contoursPoly[i][3].x, (float)contoursPoly[i][3].y };

			int contourWidth = tr.x - tl.x;
			int contourHeight = bl.y - tl.y;

			cv::Point2f src[4] = { tl, bl, br, tr };
			if (contourHeight < contourWidth)
			{
				src[0] = tr;
				src[1] = tl;
				src[2] = bl;
				src[3] = br;
			}

			cv::Point2f dst[4] = { {0.0f, 0.0f}, {0.0f, h}, {w, h}, {w, 0.0f} };

			cv::Mat mat, isolatedCardImage;
			mat = cv::getPerspectiveTransform(src, dst);

			cv::warpPerspective(originalImage, isolatedCardImage, mat, cv::Point(w, h));

			cards.push(isolatedCardImage);
		}
	}
}

cv::Mat CardsRecognizer::isolateElement(cv::Mat& image) const
{
	cv::Mat element;

	cv::Mat imageForContours;
	image.copyTo(imageForContours);

	preProcessForContoursFinding(imageForContours);

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;

	cv::findContours(imageForContours, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	for (size_t i = 0; i < contours.size(); i++)
	{
		double area = cv::contourArea(contours[i]);
		if (area > recognitionParams.minElementContourArea)
		{
			cv::Rect boundingBox = cv::boundingRect(contours[i]);

			element = image(cv::Range(boundingBox.y, boundingBox.y + boundingBox.height), cv::Range(boundingBox.x, boundingBox.x + boundingBox.width));
		}
	}

	return element;
}

CardCode CardsRecognizer::findMatch(cv::Mat& image, const ReferenceSet referenceSet, const std::string& debugWindowTitle)
{
	int smallestAmountOfWhitePixels = std::numeric_limits<int>::max();
	CardCode bestMatch = CardCode::UNDEFINED;
	int whitePixels = 0;

	for (size_t i = 0; i < referenceSet.images.size(); i++)
	{
		cv::Mat result;
		cv::multiply(referenceSet.images[i], image, result);

		int whitePixels = 0;
		for (int x = 0; x < result.rows; x++)
		{
			for (int y = 0; y < result.cols; y++)
			{
				uchar color = result.at<uchar>(x, y);
				if (color > 0)
				{
					whitePixels++;
				}
			}
		}

		if (whitePixels < smallestAmountOfWhitePixels)
		{
#if _DEBUG
			cv::imshow(debugWindowTitle, result);
#endif
			smallestAmountOfWhitePixels = whitePixels;
			bestMatch = referenceSet.imageCodes[i];
		}

	}

	return bestMatch;
}

void CardsRecognizer::processVideoInput()
{
	recognizedCards.clear();

	CardCode currentRank = CardCode::UNDEFINED;
	CardCode currentSuit = CardCode::UNDEFINED;

	cap.read(frame);
	frame.copyTo(originalFrame);

	preProcessForContoursFinding(frame);

	isolateCards(frame, originalFrame, recognitionParams.isolatedCardWith, recognitionParams.isolatedCardHeight, cards);
	while (cards.size() > 0)
	{
#if _DEBUG
		char str[256] = "";
		std::snprintf(str, sizeof str, "%zu", cards.size());
		std::string id = str;
#endif
		cv::Mat card = cards.front();
		cards.pop();

#if _DEBUG
		cv::imshow("Isolated Card " + id, card);
#endif
		cv::Mat rankCut = card(cv::Range(recognitionParams.rankCutRowsStart, recognitionParams.rankCutRowsEnd),
							   cv::Range(recognitionParams.rankCutColsStart, recognitionParams.rankCutColsEnd));
#if _DEBUG
		cv::imshow("Rank Cut " + id, rankCut);
#endif

		cv::Mat isolatedRank = isolateElement(rankCut);

		if (isolatedRank.data != nullptr)
		{
			cv::resize(isolatedRank, isolatedRank, cv::Size(ranks.imageWidth, ranks.imageHeight));
			preProcessForRecognition(isolatedRank);

			std::string rankFindMatchDebugWindowTitle = "";
#if _DEBUG
			cv::imshow("Isolated Rank " + id, isolatedRank);
			rankFindMatchDebugWindowTitle = "Best Rank Match " + id;
#endif
			currentRank = findMatch(isolatedRank, ranks, rankFindMatchDebugWindowTitle);
		}

		cv::Mat suitCut = card(cv::Range(recognitionParams.suitCutRowsStart, recognitionParams.suitCutRowsEnd),
							   cv::Range(recognitionParams.suitCutColsStart, recognitionParams.suitCutColsEnd));
#if _DEBUG
		cv::imshow("Suit Cut " + id, suitCut);
#endif

		cv::Mat isolatedSuit = isolateElement(suitCut);
		if (isolatedSuit.data != nullptr)
		{
			cv::resize(isolatedSuit, isolatedSuit, cv::Size(suits.imageWidth, suits.imageHeight));
			preProcessForRecognition(isolatedSuit);

			std::string suitFindMatchDebugWindowTitle = "";
#if _DEBUG
			cv::imshow("Isolated Suit " + id, isolatedSuit);
			suitFindMatchDebugWindowTitle = "Best Suit Match " + id;
#endif
			currentSuit = findMatch(isolatedSuit, suits, suitFindMatchDebugWindowTitle);
		}

		recognizedCards.push_back((CardCode)(currentRank | currentSuit));
	}

	cv::imshow("Cards Recognizer", originalFrame);
	cv::waitKey(1);
}

std::vector<CardCode> CardsRecognizer::getRecognizedCards() const
{
	return recognizedCards;
}


CardsRecognizer::~CardsRecognizer()
{
	cv::destroyAllWindows();
}