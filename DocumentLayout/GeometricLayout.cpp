#include "GeometricLayout.h"

namespace GeometricLayout
{
	inline float EuclidianDistance(int x1, int y1, int x2, int y2)
	{
		return sqrt((float)(x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
	}

	inline int ManhattanDistance(int x1, int y1, int x2, int y2)
	{
		int a = (x1 - x2);
		int b = (y1 - y2);
		
		if (a < 0)
			a = -a;

		if (b < 0)
			b = -b;

		return a * b;
	}

	inline int Distance(int x1, int y1, int x2, int y2)
	{
		return (int)EuclidianDistance(x1, y1, x2, y2);
	}

	int RectDistance2(Rect a, Rect b)
	{
		int x1 = a.x;
		int x1b = a.x + a.width;
		int y1 = a.y;
		int y1b = a.y + a.height;

		int x2 = b.x;
		int x2b = b.x + b.width;
		int y2 = b.y;
		int y2b = b.y + b.height;

		bool left = x2b < x1;
		bool right = x1b < x2;
		bool bottom = y2b < y1;
		bool top = y1b < y2;

		int min_distance = INT_MAX;

		int dist = Distance(x1, y1, x2, y2);
		if (dist < min_distance)
			min_distance = dist;

		dist = Distance(x1, y1b, x2, y2);
		if (dist < min_distance)
			min_distance = dist;

		dist = Distance(x1b, y1, x2, y2);
		if (dist < min_distance)
			min_distance = dist;

		dist = Distance(x1b, y1b, x2, y2);
		if (dist < min_distance)
			min_distance = dist;
		////
		dist = Distance(x1, y1b, x2b, y2b);
		if (dist < min_distance)
			min_distance = dist;

		dist = Distance(x1, y1b, x2b, y2b);
		if (dist < min_distance)
			min_distance = dist;

		dist = Distance(x1b, y1, x2b, y2b);
		if (dist < min_distance)
			min_distance = dist;

		dist = Distance(x1b, y1b, x2b, y2b);
		if (dist < min_distance)
			min_distance = dist;
		////
		dist = Distance(x1, y1b, x2, y2b);
		if (dist < min_distance)
			min_distance = dist;

		dist = Distance(x1, y1b, x2, y2b);
		if (dist < min_distance)
			min_distance = dist;

		dist = Distance(x1b, y1, x2, y2b);
		if (dist < min_distance)
			min_distance = dist;

		dist = Distance(x1b, y1b, x2, y2b);
		if (dist < min_distance)
			min_distance = dist;
		////
		dist = Distance(x1, y1b, x2b, y2);
		if (dist < min_distance)
			min_distance = dist;

		dist = Distance(x1, y1b, x2b, y2);
		if (dist < min_distance)
			min_distance = dist;

		dist = Distance(x1b, y1, x2b, y2);
		if (dist < min_distance)
			min_distance = dist;

		dist = Distance(x1b, y1b, x2b, y2);
		if (dist < min_distance)
			min_distance = dist;

		return min_distance;
	}

	inline float ImageBlackFactor(Mat img)
	{
		const int nz = countNonZero(img);
		const int total = img.rows * img.cols;

		return (float)nz / total;
	}

	int RectDistance(Rect a, Rect b)
	{
		int x1 = a.x;
		int x1b = a.x + a.width;
		int y1 = a.y;
		int y1b = a.y + a.height;

		int x2 = b.x;
		int x2b = b.x + b.width;
		int y2 = b.y;
		int y2b = b.y + b.height;

		bool left = x2b < x1;
		bool right = x1b < x2;
		bool bottom = y2b < y1;
		bool top = y1b < y2;

		if (top && left)
			return Distance(x1, y1b, x2b, y2);
		else if (left && bottom)
			return Distance(x1, y1, x2b, y2b);
		else if (bottom && right)
			return Distance(x1b, y1, x2, y2b);
		else if (right && top)
			return Distance(x1b, y1b, x2, y2);
		else if (left)
			return abs(x1 - x2b);
		else if (right)
			return abs(x2 - x1b);
		else if (bottom)
			return abs(y1 - y2b);
		else if (top)
			return abs(y2 - y1b);
		else
			return 0;
		//return RectDistance2(a,b);
	}

	bool UpdateBoundingRects(Mat img, vector<Rect>& rects, vector<Rect>& finalRects, int D)
	{
		bool done = true;

		vector<Rect> newRects;

		Rect newRect;

		vector<Rect> nextSearchRect;
		vector<Rect> searchRect = rects;
		bool findMore = false;

		newRect = searchRect[0];
		float currentFactor = ImageBlackFactor(img(newRect));

		do
		{
			findMore = false;
			nextSearchRect.clear();
			bool addToNextSearch = true;
			for (int i = 1; i < searchRect.size(); i++)
			{
				int dist = RectDistance(newRect, searchRect[i]);
				if (dist <= D)
				{
					float nextFactor = ImageBlackFactor(img(newRect | searchRect[i]));

					if (abs(nextFactor - currentFactor) < 0.2)
					{
						done = false;
						newRect |= searchRect[i];
						currentFactor = ImageBlackFactor(img(newRect));
						findMore = true;
						addToNextSearch = false;
					}
				}
				else
				{
					nextSearchRect.push_back(searchRect[i]);
				}
			}
			searchRect = nextSearchRect;
		} while (findMore);

		finalRects.push_back(newRect);

		rects = searchRect;

		return rects.size() == 0;
	}

	void DrawBoundingRects(Mat src, vector<Rect> rects, Scalar color)
	{
		for (int i = 0; i < rects.size(); i++)
		{
			Rect brect = rects[i];
			rectangle(src, brect, color, 1);
		}
	}

	std::vector<Rect> DetectLayout(Mat src)
	{
		Mat cannyOutput;
		Canny(src, cannyOutput, 100, 200);

#ifdef _DEBUG
		imshow("Canny", cannyOutput);
#endif // _DEBUG
		//waitKey(0);

		/*Mat element = getStructuringElement(MORPH_RECT, Size(3, 3), Point(1, 1));
		dilate(cannyOutput, cannyOutput, element);*/


		Mat blurOutput;
		blur(cannyOutput, blurOutput, Size(5, 5));

		/*imshow("Blur", blurOutput);
		waitKey(0);*/

		//Contours
		vector <vector<Point>> contours;
		vector<Vec4i> hierarchy;

		Mat contoursImg;
		src.copyTo(contoursImg);

		findContours(cannyOutput, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE);

		Scalar incolor = Scalar(0, 0, 255);
		Scalar outcolor = Scalar(255, 0, 0);
		vector<Rect> boundingRects;
		for (int i = 0; i < contours.size(); i++)
		{
			Scalar color = outcolor;
			Rect brect = boundingRect(contours[i]);
			boundingRects.push_back(brect);
			rectangle(contoursImg, brect, color, 1);
		}

		imshow("Contours", contoursImg);
		Mat layoutImg;
		Mat thrshImg;
		cvtColor(src, thrshImg, CV_BGR2GRAY);
		threshold(thrshImg, thrshImg, 100, 255, THRESH_BINARY);

		vector<Rect> newBoundingRects;
		while (!UpdateBoundingRects(thrshImg, boundingRects, newBoundingRects, 10))
		{
#ifdef _DEBUG
			waitKey(10);
			src.copyTo(layoutImg);
			DrawBoundingRects(layoutImg, newBoundingRects, outcolor);
			imshow("Contours - Layout", layoutImg);
#endif // _DEBUG
		}

#ifdef _DEBUG
		waitKey(500);
		src.copyTo(layoutImg);
		DrawBoundingRects(layoutImg, newBoundingRects, outcolor);
		imshow("Contours - Layout", layoutImg);

		waitKey(0);
#endif // _DEBUG

		return newBoundingRects;
	}
}
