#include "HeuristicLayout.h"

namespace HeuristicLayout
{
	using namespace cv;

	float getStdDev(const Mat& frame, const InputOutputArray& mask = noArray())
	{
		Mat mean;
		Mat stddev;

		meanStdDev(frame, mean, stddev, mask);

		return stddev.at<double>(0, 0);
	}

	float getEntropyFromHist(const Mat& hist)
	{
		Mat _hist = hist.clone();
		const size_t total = sum(hist).val[0];
		if (total == 0)
			return 0.f;

		_hist /= total;

		Mat logP;
		cv::log(_hist, logP);
		logP.forEach<float>([](float &p, const int * position) -> void {
			if (isinf(p))
				p = 0.f;
		});

		float entropy = -1 * sum(_hist.mul(logP)).val[0];
		assert(!(entropy < 0.0f));

		return entropy;
	}

	void /*ImageCut*/ getBestCut(Mat &originalImage, unsigned level)
	{
		unsigned long minQuote = std::numeric_limits<unsigned long>::max();
		Mat minS[4];

		//ImageCut ret;

		std::vector<Vec4i> lines;
		std::vector<unsigned> xs, ys;
		xs.push_back(0); ys.push_back(0);
		//xs.push_back(originalImage.cols / 2); ys.push_back( originalImage.rows / 2);

#ifdef _DEBUG
		Mat color_dst;
		cvtColor(originalImage, color_dst, CV_GRAY2BGR);
#endif // _DEBUG

		float sizeAvg = std::min(originalImage.rows, originalImage.cols);

		HoughLinesP(originalImage, lines, 1, CV_PI / 2, std::min(350.0, sizeAvg * pow(0.68f, level)), std::min(30.0, sizeAvg * 0.06), 10.0);
		for (size_t i = 0; i < lines.size(); i++)
		{
#ifdef _DEBUG
			line(color_dst, Point(lines[i][0], lines[i][1]),
				Point(lines[i][2], lines[i][3]), Scalar(0, 255, 0), 1, 8);
#endif // _DEBUG

			if (lines[i][0] == lines[i][2])
			{
				xs.push_back(lines[i][0]);
				continue;
			}

			if (lines[i][1] == lines[i][3])
			{
				ys.push_back(lines[i][1]);
				continue;
			}
		}

		for (int y : ys)
		{
			for (int x : xs)
			{
				Mat S[] = {
					originalImage(Rect(0, 0, x, y)),
					originalImage(Rect(x, 0, originalImage.cols - x, y)),
					originalImage(Rect(0, y, x, originalImage.rows - y)),
					originalImage(Rect(x, y, originalImage.cols - x, originalImage.rows - y)),
				};

				unsigned long quote = 0;
				std::vector <unsigned char> huffData[4];
				for (unsigned i = 0; i < 4; i++)
				{
					const size_t totalSize = S[i].total();
					if (totalSize == 0)
						continue;

					huffData[i].resize(totalSize * 2); // TODO: Investigate larger Huffman

					size_t huffSz = 0;// Huffman_Compress(S[i].ptr(), huffData[i].data(), totalSize);
					huffData[i].resize(huffSz);
					quote += huffSz;
				}

				if (quote < minQuote)
				{
					minQuote = quote;
// 					ret.point.x = x;
// 					ret.point.y = y;
//					ret.totalSize = quote;
// 					for (unsigned i = 0; i < 4; i++)
// 						ret.data[i] = huffData[i];

					minS[0] = S[0];
					minS[1] = S[1];
					minS[2] = S[2];
					minS[3] = S[3];
				}
			}
		}

// #ifdef _DEBUG
// 		line(color_dst, Point(ret.point.x, 0), Point(ret.point.x, originalImage.rows), Scalar(0, 0, 255), 2, 8);
// 		line(color_dst, Point(0, ret.point.y), Point(originalImage.cols, ret.point.y), Scalar(0, 0, 255), 2, 8);
// #endif // _DEBUG

		//return ret;
	}

	float getEntropy(const Mat& frame, const Mat& mask = Mat())
	{
		const size_t total = mask.empty() ? frame.total() : countNonZero(mask);

		if (total == 0)
			return 0.f;

		const int histSize = 256;
		/// Set the ranges ( for B,G,R) )
		const float range[] = { 0, 256 };
		const float* histRange = { range };

		Mat hist;

		/// Compute the histograms:
		calcHist(&frame, 1, 0, mask, hist, 1, &histSize, &histRange);
		hist /= total;

		Mat logP;
		cv::log(hist, logP);
		logP.forEach<float>([](float &p, const int * position) -> void {
			if (isinf(p))
				p = 0.f;
		});

		float entropy = -1 * sum(hist.mul(logP)).val[0];

		return entropy;
	}

	Mat GetMask(const Mat &subMat, const vector<Point> &contour)
	{
		Mat mask(subMat.size(), CV_8U, Scalar(0));

		Size origSize;
		Point offset;
		subMat.locateROI(origSize, offset);

		auto offsetPoints = contour;
		for (auto& p : offsetPoints)
			p -= offset;

		fillConvexPoly(mask, offsetPoints, Scalar(255));

		return mask;
	}

	std::vector<Rect> DetectLayout(Mat document)
	{
		Mat original = document.clone();
		Mat originalGray;

		if (document.channels() != 1)
		{
			document = document.clone();
			cvtColor(document, document, CV_BGR2GRAY);
		}
		
		originalGray = document.clone();

		Mat originalThr;
		threshold(originalGray, originalThr, 128, 255, 1);

		float sizeMaxRatio = 256.f / (float)std::max(document.rows, document.cols);

		// downsample for faster and better processing
		resize(document, document, Size(), sizeMaxRatio, sizeMaxRatio, INTER_CUBIC);
		//blur(document, document, Size(3, 3));

		Mat thrMat;
		threshold(document, thrMat, 240, 255, 1);

		Mat thrMatFull;
		threshold(originalGray, thrMatFull, 240, 255, 1);

		Mat element = getStructuringElement(MORPH_RECT, Size(3, 3), Point(1, 1));
		Mat elementCr = getStructuringElement(MORPH_CROSS, Size(3, 3), Point(1, 1));
		Mat element3 = getStructuringElement(MORPH_CROSS, Size(9, 9), Point(4, 4));
		Mat elementSq = getStructuringElement(MORPH_RECT, Size(9, 9), Point(4, 4));

		Mat cannyOut;
		Canny(original, cannyOut, 150, 200);
		
		cannyOut.convertTo(cannyOut, CV_8U);

		dilate(cannyOut, cannyOut, element3);
		//erode(cannyOut, cannyOut, elementSq);

		Mat cannySmall;
		resize(cannyOut, cannySmall, Size(), sizeMaxRatio, sizeMaxRatio, INTER_NEAREST);
		dilate(cannySmall, cannySmall, element);
		erode (cannySmall, cannySmall, element);

		//////////////////////////////////////////////////////////////////////////
		//thrMat = cannySmall;
		//////////////////////////////////////////////////////////////////////////

		Mat cannyOutDraw = cannyOut.clone();
		cvtColor(cannyOutDraw, cannyOutDraw, CV_GRAY2BGR);

		{
			vector <vector<Point>> contours;
			vector<Vec4i> hierarchy;
			//findContours(morpthMat, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
			findContours(cannyOut, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

			for (int i = contours.size()-1; i >= 0; i--)
			{
				const vector<Point> &contour = contours[i];
// 				if (contour.size() < 3)
// 					continue;

				Rect brect = boundingRect(contour);

#ifdef _DEBUG
				rectangle(color_dst, brect, outcolor, 1);
#endif // _DEBUG

				Rect smallRect(brect.tl() * sizeMaxRatio, brect.br() * sizeMaxRatio);

				if (smallRect.width == 0 || smallRect.height == 0)
					continue;

				drawContours(cannyOutDraw, contours, i, Scalar(255,0,255), 2, 8, hierarchy, 0, Point());

				for (int j = 0; j < contour.size(); j++)
					contours[i][j] *= sizeMaxRatio;

				Mat subMat = cannySmall(smallRect);
				Mat subMatDoc = document(smallRect);
				Mat mask = GetMask(subMat, contour);

				Mat subMatThr = thrMat(smallRect);
				Mat compositeMask;
 				//bitwise_or(mask, subMat, compositeMask);
 				bitwise_and(mask, subMatThr, compositeMask);
				//compositeMask = mask;

				const float entropy = getEntropy(subMatDoc, compositeMask);
				const float stddev = getStdDev(subMatDoc, compositeMask);
				
				if (entropy > 2.5f)
				{
					bitwise_xor(subMatThr, subMatThr, subMatThr, compositeMask);
				}
			}
		}

		Mat morpthMat = thrMat.clone();
		dilate(morpthMat, morpthMat, element);

		bitwise_and(morpthMat, cannySmall, morpthMat);
		erode(morpthMat, morpthMat, elementCr);

		const float sizeMin = std::min(document.rows, document.cols);
		const float sizeMax = std::max(document.rows, document.cols);
		std::vector<Rect> ret;

#ifdef _DEBUG
		Mat color_dst;
		cvtColor(document, color_dst, CV_GRAY2BGR);
#endif // _DEBUG

		vector <vector<Point>> contours;
		vector<Vec4i> hierarchy;
		findContours(morpthMat, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
		//findContours(thrMat, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

		Scalar outcolor = Scalar(255, 0, 0);

		for (int i = 0; i < contours.size(); i++)
		{
			Scalar color = outcolor;
			Rect brect = boundingRect(contours[i]);
			
#ifdef _DEBUG
			rectangle(color_dst, brect, outcolor, 1);
#endif // _DEBUG

// 			Mat subMat = thrMat(brect);
// 			Mat subMatDoc = document(brect);
// 			Mat mask = GetMask(subMat, contours[i]);
// 			
// 			const float entropy = getEntropy(subMatDoc, mask);

			for (int j = 0; j < contours[i].size(); j++)
				contours[i][j] /= sizeMaxRatio;

			//drawContours(original, contours, i, color, 2, 8, hierarchy, 0, Point());

			brect.x /= sizeMaxRatio;
			brect.y /= sizeMaxRatio;
			brect.width /= sizeMaxRatio;
			brect.height /= sizeMaxRatio;
			rectangle(original, brect, outcolor, 2);

			ret.emplace_back(brect);
		}

#ifdef _DEBUG
		imshow("Layout", original);
		waitKey(0);
#endif // _DEBUG

		return ret;
	}


#pragma region HOUGH LINES OLD
	// 		HoughLinesP(document, lines, 1.0, CV_PI / 2.0, sizeMin / 4.0, 4);
	// 		//HoughLines(document, lines, 1.0, CV_PI / 4.0, 1, 1, 1, 0, CV_PI / 2.0);
	// 
	// 		for (size_t i = 0; i < lines.size(); i++)
	// 		{
	// #ifdef _DEBUG
	// 			Point start, end;
	// 
	// 			start = Point(lines[i][0], lines[i][1]);
	// 			end   = Point(lines[i][2], lines[i][3]);
	// 
	// // 			if (abs(lines[i][1] - CV_PI / 2.0) < 0.00001f) // horizontal
	// // 			{
	// // 				start.x = 0;
	// // 				start.y = lines[i][0];
	// // 				end.x = document.cols;
	// // 				end.y = lines[i][0];
	// // 			}
	// // 			else if (abs(lines[i][1]) < 0.00001f) // vertical
	// // 			{
	// // 				start.x = lines[i][0];
	// // 				start.y = 0;
	// // 				end.x = lines[i][0];
	// // 				end.y = document.rows;
	// // 			}
	// 
	// 			line(color_dst, start, end, Scalar(0, 255, 0), 1, 8);
	// #endif // _DEBUG
	// 
	// 			imshow("Lines", color_dst);
	//			waitKey(100);
	// 
	// 			if (start.x == end.x)
	// 			{
	// 				xs.push_back(lines[i][0]);
	// 				continue;
	// 			}
	// 
	// 			if (start.y == end.y)
	// 			{
	// 				ys.push_back(lines[i][1]);
	// 				continue;
	// 			}
	// 		}
#pragma endregion HOUGH LINES OLD
}