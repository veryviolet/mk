#ifndef CMARKLOGO_H
#define CMARKLOGO_H

#include <string>

using namespace std;

#include <xmarker/mark.h>

#include <opencv2/videostab.hpp>
#include <opencv2/videostab/inpainting.hpp>

using namespace cv::videostab;

class CMarkLogo : public CMark
{
	Mat LogoImage;
	Mat LogoMask;

	double Threshold;

	double Alpha;

	ColorAverageInpainter Inpainter;

public:
	bool ProcessFrame(Mat & pFrame);
	CMarkLogo();
	~CMarkLogo();
	bool SetLogoImage(const char * fname);
	void SetAlpha(double alpha);
};

#endif
