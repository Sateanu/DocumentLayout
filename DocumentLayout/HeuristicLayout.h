#pragma once

#include <vector>
#include <opencv2/opencv.hpp>

#include "config.h"


namespace HeuristicLayout
{
	using namespace cv;
	using namespace std;

	vector<Rect> DetectLayout(Mat document);
}
