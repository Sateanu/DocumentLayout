#include <iostream>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>

#include "HeuristicLayout.h"

using namespace cv;
using namespace std;

int main()
{
	Mat src = imread("pic2.png");
	
	if (src.empty())
	{
		cout << "Can't find image";
		return -1;
	}

	imshow("Original", src);
	//waitKey(0);

	HeuristicLayout::DetectLayout(src);
	waitKey(0);

	Mat cannyOutput;
	Canny(src, cannyOutput, 100, 200);

	imshow("Canny", cannyOutput);
	waitKey(0);

	Mat blurOutput;
	blur(cannyOutput, blurOutput, Size(5, 5));

	imshow("Blur", blurOutput);
	waitKey(0);
	
	//Contours
	vector <vector<Point>> contours;
	vector<Vec4i> hierarchy;

	Mat contoursImg(src);
	findContours(cannyOutput, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE);

	Scalar incolor = Scalar(0, 255, 255);
	Scalar outcolor = Scalar(255, 0, 0);

	for (int i = 0; i < contours.size(); i++)
	{
		Scalar color = outcolor;
		Rect brect = boundingRect(contours[i]);
		rectangle(contoursImg, brect, outcolor, 1);
	}

	imshow("Contours", contoursImg);
	waitKey(0);

	destroyAllWindows();
	return 0;
}