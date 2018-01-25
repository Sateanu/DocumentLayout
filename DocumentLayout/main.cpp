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

	FILE* stats = fopen("stats.csv", "w");
	fprintf(stats, "FileName,HeuristicRuntime,GeometricRuntime,VoronoiRuntime,MajorityScore,UnanimityScore\n");

	for (int i = 1; i < argc; i++)
	{
		const string fullName = argv[i];
		const size_t lastDot = fullName.find_last_of('.');
		const string extension = fullName.substr(lastDot);
		const string baseName = fullName.substr(0, lastDot);
		
		Mat src = imread(fullName);

		fprintf(stats, "%s,", baseName.c_str());

		if (src.empty())
		{
			printf("Can't find image: %s. Skipping...", argv[i]);
			continue;
		}

#ifdef _DEBUG
		imshow("Original", src);
#endif // _DEBUG

		Mat originalGray = src.clone();

		if (originalGray.channels() != 1)
			cvtColor(originalGray, originalGray, CV_BGR2GRAY);

		Mat originalThr;
		threshold(originalGray, originalThr, 128, 255, 1);

		originalThr.convertTo(originalThr, CV_8UC1);

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
			fprintf(stats, "%lf,", t);

			results.emplace_back(r);
		}

// 		for (auto& v : results)
// 		{
// 			Mat m = Mat::zeros(src.rows, src.cols, CV_8UC1);
// 			for (auto &r : v)
// 			{
// 				if (r.height == 0 || r.width == 0)
// 					continue;
// 
// 				m(r) += Mat::ones(r.height, r.width, CV_8UC1) * 255;
// 			}
// 		}

		Mat voting = Mat::zeros(src.rows, src.cols, CV_8UC1);

		Mat toDraw = src.clone();
		RNG rng(12345);

		// TODO: add weights
		vector<Voting::VotingResult> all = Voting::MergeRects(results, thresh);
		for (auto& r : all)
		{
			if (r.r.height == 0 || r.r.width == 0)
				continue;

			if (r.r.x + r.r.width > voting.cols)
				r.r.width = voting.cols - r.r.x;

			if (r.r.y + r.r.height > voting.rows)
				r.r.height = voting.rows - r.r.y;

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

		const size_t lastDotBase   = baseName.find_last_of('.');
		const string extensionBase = baseName.substr(lastDotBase);
		const string baseNameBase  = baseName.substr(0, lastDotBase);

		assert(extensionBase == ".orig");

		Mat cleanMajority;
		Mat cleanUnanimity;

		bitwise_and(votingMajority, originalThr, cleanMajority);
		bitwise_and(votingUnanimity, originalThr, cleanUnanimity);

		Mat groundTruth = imread(baseNameBase + ".clean.png", IMREAD_GRAYSCALE);

		Mat diffMajority;
		Mat diffUnanimity;

		bitwise_xor(groundTruth, cleanMajority, diffMajority);
		bitwise_xor(groundTruth, cleanUnanimity, diffUnanimity);

		float scoreMajority = ((float)diffMajority.total() - countNonZero(diffMajority)) / (float)diffMajority.total();
		float scoreUnanimity = ((float)diffUnanimity.total() - countNonZero(diffUnanimity)) / (float)diffUnanimity.total();

		fprintf(stats, "%lf,%lf\n", scoreMajority, scoreUnanimity);
	}

	fclose(stats);

	puts("Press any key to exit...");
	getchar();

	return 0;
}