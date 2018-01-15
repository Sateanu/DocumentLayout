#include <iostream>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <Windows.h>
#include "HeuristicLayout.h"
#include "GeometricLayout.h"
#include "VoronoiLayout.h"

using namespace cv;
using namespace std;

int main()
{
	Mat src = imread("pic1.png");
	
	if (src.empty())
	{
		cout << "Can't find image";
		return -1;
	}

	imshow("Original", src);
	//waitKey(0);

	//HeuristicLayout::DetectLayout(src);
	//GeometricLayout::DetectLayout(src);
	VoronoiLayout::DetectLayout(src);



	destroyAllWindows();
	return 0;
}