#pragma once

#include <vector>
#include <opencv2/opencv.hpp>

#include "config.h"

namespace GeometricLayout
{
	using namespace std;
	using namespace cv;

// 	int Distance(int x1, int y1, int x2, int y2);
// 	int RectDistance(Rect a, Rect b);
// 	int RectDistance2(Rect a, Rect b);
// 	float ImageBlackFactor(Mat img);
// 	bool UpdateBoundingRects(Mat img, vector<Rect>& rects, vector<Rect>& finalRects, int D);
// 	void DrawBoundingRects(Mat src, vector<Rect> rects, Scalar color);

	vector<Rect> DetectLayout(Mat src);
}