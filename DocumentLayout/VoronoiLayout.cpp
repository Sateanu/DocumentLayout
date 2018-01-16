#include "VoronoiLayout.h"

namespace VoronoiLayout
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
	void draw_point(Mat& img, Point2f fp, Scalar color)
	{
		circle(img, fp, 2, color, CV_FILLED, CV_AA, 0);
	}

	// Draw delaunay triangles
	void draw_delaunay(Mat& img, Subdiv2D& subdiv, Scalar delaunay_color)
	{

		vector<Vec6f> triangleList;
		subdiv.getTriangleList(triangleList);
		vector<Point> pt(3);
		Size size = img.size();
		Rect rect(0, 0, size.width, size.height);

		for (size_t i = 0; i < triangleList.size(); i++)
		{
			Vec6f t = triangleList[i];
			pt[0] = Point(cvRound(t[0]), cvRound(t[1]));
			pt[1] = Point(cvRound(t[2]), cvRound(t[3]));
			pt[2] = Point(cvRound(t[4]), cvRound(t[5]));

			// Draw rectangles completely inside the image.
			if (rect.contains(pt[0]) && rect.contains(pt[1]) && rect.contains(pt[2]))
			{
				line(img, pt[0], pt[1], delaunay_color, 1, CV_AA, 0);
				line(img, pt[1], pt[2], delaunay_color, 1, CV_AA, 0);
				line(img, pt[2], pt[0], delaunay_color, 1, CV_AA, 0);
			}
		}
	}

	//Draw voronoi diagram
	void draw_voronoi(Mat& img, Subdiv2D& subdiv)
	{
		vector<vector<Point2f> > facets;
		vector<Point2f> centers;
		subdiv.getVoronoiFacetList(vector<int>(), facets, centers);
		
		vector<Point> ifacet;
		vector<vector<Point> > ifacets(1);

		float thresh = 4;

		for (size_t i = 0; i < facets.size(); i++)
		{
			
			//ifacet.resize(facets[i].size());
			ifacet.clear();

			ifacet.push_back(facets[i][0]);
			for (size_t j = 1; j < facets[i].size(); j++)
			{
				const Point &p = facets[i][j];
				if (norm(p - *ifacet.rbegin()) < thresh)
					ifacet.push_back(p);
			}

			ifacets[0] = ifacet;
			//polylines(img, ifacets, false, Scalar(), 1, CV_AA, 0);
			fillPoly(img, ifacets, Scalar(255));
		}
	}

	float voronoiArea()
	{
		return 0.0f;
	}

	std::vector<Rect> DetectLayout(Mat src)
	{
		Mat cannyOutput;
		Mat grayScale;
		cvtColor(src, grayScale, CV_BGR2GRAY);


		Rect rect(0, 0, src.size().width, src.size().height);
		Subdiv2D subdiv(rect);
		Canny(src, cannyOutput, 100, 200);

		//HoughLines(grayScale, cannyOutput, CV_PI / 180, 15, 100);

		
#ifdef _DEBUG
		imshow("Canny", cannyOutput);
#endif // _DEBUG
		//waitKey(0);

		//Contours
		vector <vector<Point>> contours;
		vector<Vec4i> hierarchy;
		Mat contoursImg;
		src.copyTo(contoursImg);
		
		findContours(cannyOutput, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

		Scalar incolor = Scalar(0, 0, 255);
		Scalar outcolor = Scalar(255, 0, 0);
		Scalar black = Scalar(0,0 , 0);
		vector<Rect> boundingRects;
		for (int i = 0; i < contours.size(); i++)
		{
			Scalar color = outcolor;
			Rect brect = boundingRect(contours[i]);
			boundingRects.push_back(brect);
			rectangle(contoursImg, brect, color, 1);
			for (int j = 0; j < contours[i].size(); j++)
			{
				subdiv.insert(Point2f(contours[i][j].x, contours[i][j].y));
			}
		}

#ifdef _DEBUG
		imshow("Contours", contoursImg);
#endif // _DEBUG
		Mat delauney = Mat::zeros(grayScale.size(), grayScale.type());
		draw_voronoi(delauney, subdiv);

		//Scalar delaunay_color(255, 255, 255), points_color(0, 0, 255);
		//draw_delaunay(delauney, subdiv, delaunay_color);

		//imshow("Delauney", delauney);

		Mat element = getStructuringElement(MORPH_RECT, Size(3, 3), Point(1, 1));
		Mat element1 = getStructuringElement(MORPH_RECT, Size(5, 5), Point(2, 2));
		//threshold(delauney, delauney, 240, 255, 1);
		erode(delauney, delauney, element);
		dilate(delauney, delauney, element1);

		/*Mat layoutImg;
		Mat thrshImg;
		cvtColor(src, thrshImg, CV_BGR2GRAY);
		threshold(thrshImg, thrshImg, 100, 255, THRESH_BINARY);*/

		vector<Rect> newBoundingRects;
		while (!UpdateBoundingRects(delauney, boundingRects, newBoundingRects, 20));

#ifdef _DEBUG
		waitKey(0);
#endif // _DEBUG

		return newBoundingRects;
	}
}
