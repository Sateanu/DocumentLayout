#include <cstdio>
#include <functional>

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

typedef vector<Rect> AlgorithmResult;
typedef function<AlgorithmResult(Mat)> Algorithm;

struct AlgorithmDesc
{
	::Algorithm algo;
	string desc;
};

struct MatDesc
{
	Mat m;
	string desc;
};

#define Desc(a) {a, #a}

double PCFreq = 0.0;
__int64 CounterStart = 0;

void StartCounter()
{
	LARGE_INTEGER li;
	if (!QueryPerformanceFrequency(&li))
		puts("QueryPerformanceFrequency failed!");

	PCFreq = double(li.QuadPart) / 1000.0;

	QueryPerformanceCounter(&li);
	CounterStart = li.QuadPart;
}

double GetCounter()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return double(li.QuadPart - CounterStart) / PCFreq;
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("Usage:\n\t%s image1 [image2] ...\n", argv[0]);
		return -1;
	}

	for (int i = 1; i < argc; i++)
	{
		const string fullName = argv[i];
		const size_t lastDot = fullName.find_last_of('.');
		const string extension = fullName.substr(lastDot);
		const string baseName = fullName.substr(0, lastDot);
		
		Mat src = imread(fullName);

		if (src.empty())
		{
			printf("Can't find image: %s. Skipping...", argv[i]);
			continue;
		}

#ifdef _DEBUG
		imshow("Original", src);
#endif // _DEBUG

		const float thresh = 0.01f * min(src.cols, src.rows); // 1% of smallest edge
		
		vector<::AlgorithmResult> results;
		vector<::AlgorithmDesc> algorithms{
			Desc(HeuristicLayout::DetectLayout),
			Desc(GeometricLayout::DetectLayout),
			Desc(VoronoiLayout::DetectLayout)
		};

		for (auto &a : algorithms)
		{
			StartCounter();
			const vector<Rect> r = a.algo(src);
			double t = GetCounter();

			printf("Algorithm %s took %lf ms for image %s\n", a.desc.c_str(), t, argv[i]);

			results.emplace_back(r);
		}

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

		// scale results:
		//   0 votes black
		// 255 all votes
		voting *= (255 / results.size());

		imwrite(baseName + "_voting_all" + extension, voting);

		Mat votingUnanimity, votingMajority;

		threshold(voting, votingUnanimity, 250, 255, 0); // 100%
		threshold(voting, votingMajority, 126, 255, 0); // >50%

		vector<MatDesc> votingTypes{
			Desc(votingUnanimity),
			Desc(votingMajority)
		};
		
		for (auto& vt : votingTypes)
		{
			imwrite(baseName + "_" + vt.desc + extension, votingUnanimity);

			Mat out = src.clone();
			vector <vector<Point>> contours;
			vector<Vec4i> hierarchy;
			findContours(vt.m, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

			Scalar outcolor = Scalar(255, 0, 0);
			for (int i = 0; i < contours.size(); i++)
			{
				Scalar color = outcolor;
				Rect brect = boundingRect(contours[i]);
				rectangle(out, brect, outcolor, 2);
			}

			imwrite(baseName + "_" + vt.desc + "_rects" + extension, out);
		}
	}

	puts("Press any key to exit...");
	getchar();

	return 0;
}