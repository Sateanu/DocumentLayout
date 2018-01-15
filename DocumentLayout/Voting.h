#pragma once

#include <vector>
#include <algorithm>
#include <functional>
#include <opencv2/opencv.hpp>

#include "config.h"


namespace Voting
{
	using namespace std;
	using namespace cv;

	inline bool RectsEqual(const Rect &a, const Rect &b, const float thresh)
	{
		Vec2i topleft = Vec2i(a.x, a.y) - Vec2i(b.x, b.y);
		Vec2i botleft = Vec2i(a.x, a.y + a.height) - Vec2i(b.x, b.y + b.height);
		Vec2i topright = Vec2i(a.x + a.width, a.y) - Vec2i(b.x + b.width, b.y);
		Vec2i botright = Vec2i(a.x + a.width, a.y + a.height) - Vec2i(b.x + b.width, b.y + b.height);

		double n1 = norm(topleft);
		double n2 = norm(botleft);
		double n3 = norm(topright);
		double n4 = norm(botright);

		return	n1 < thresh &&
				n2 < thresh &&
				n3 < thresh &&
				n4 < thresh;
	}

	inline Rect AverageRect(const Rect &a, const Rect &b)
	{
		return Rect((a.x + b.x) / 2, (a.y + b.y) / 2, (a.width + b.width) / 2, (a.height + b.height) / 2);
	}

	struct VotingResult
	{
		Rect r;
		unsigned c;

		VotingResult() : c(1) {}
		VotingResult(const Rect& _r)
			: r(_r)
			, c(1)
		{}
	};

	vector<VotingResult> MergeRects(vector<vector<Rect>> &detectedRects, const float thresh)
	{
		vector<VotingResult> output;

		for (auto &v : detectedRects)
		{
			output.insert(output.end(), v.begin(), v.end());
		}

		for (auto it1 = output.begin(); it1 != output.end(); ++it1)
		{
			for (auto it2 = it1 + 1; it2 != output.end();)
			{
				if (RectsEqual(it1->r, it2->r, thresh))
				{
					it1->r = AverageRect(it1->r, it2->r);
					it1->c += it2->c;
					it2 = output.erase(it2);
				}
				else
				{
					++it2;
				}
			}
		}

		return output;
	}
}