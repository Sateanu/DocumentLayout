#pragma once

#include <vector>
#include <opencv2/opencv.hpp>

namespace VoronoiLayout
{
	using namespace cv;
	using namespace std;

	vector<Rect> DetectLayout(Mat src);
}

