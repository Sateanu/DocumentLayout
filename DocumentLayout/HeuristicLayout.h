#pragma once
#include <opencv2/opencv.hpp>

namespace HeuristicLayout
{
	using namespace cv;
	using namespace std;

	vector<Rect> DetectLayout(Mat document);
}
