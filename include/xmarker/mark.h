#ifndef CMARK_H
#define CMARK_H

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include <string>
#include <queue>

using namespace std;

using namespace cv;

class CMark
{
	string Name;
	Mat LogoImage;
public:
	CMark();
	~CMark();
	void SetName(string & nm);
	virtual bool ProcessFrame(Mat & pFrame) = 0;
};

#endif
