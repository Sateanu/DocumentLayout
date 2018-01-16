#include <iostream>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <Windows.h>

#include "HeuristicLayout.h"
#include "GeometricLayout.h"
#include "VoronoiLayout.h"
#include "Voting.h"

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

	const float thresh = 0.01f * min(src.cols, src.rows); // 1% of smallest edge
	vector<vector<Rect>> results = {  HeuristicLayout::DetectLayout(src),
									  GeometricLayout::DetectLayout(src),
									  VoronoiLayout::DetectLayout(src)
									};

	for (auto& v : results)
	{
		Mat m = Mat::zeros(src.rows, src.cols, CV_8UC1);
		for (auto &r : v)
		{
			m(r) += Mat::ones(r.height, r.width, CV_8UC1) * 255;
		}
	}

	Mat voting = Mat::zeros(src.rows, src.cols, CV_8UC1);
	
	Mat toDraw = src.clone();
	RNG rng(12345);

	// TODO: add weights
	vector<Voting::VotingResult> all = Voting::MergeRects(results, thresh);
	for (auto& r : all)
	{
		voting(r.r) += Mat::ones(r.r.height, r.r.width, CV_8UC1) * r.c;

		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		rectangle(toDraw, r.r, color);
	}

	voting *= (255 / results.size());

	destroyAllWindows();
	return 0;
}