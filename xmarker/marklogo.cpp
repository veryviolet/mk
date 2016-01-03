#include <xmarker/marklogo.h>

CMarkLogo::CMarkLogo()
{
	Inpainter.setRadius(10);
	Threshold = 20;
}


CMarkLogo::~CMarkLogo()
{
}


bool CMarkLogo::SetLogoImage(const char * fname)
{
	LogoImage = imread(fname);

	cvtColor(LogoImage, LogoMask, COLOR_BGR2GRAY);

	threshold(LogoMask, LogoMask, Threshold, 255, THRESH_BINARY_INV);

	imwrite("mask.jpg", LogoMask);

	return LogoImage.data != NULL;
}

void CMarkLogo::SetAlpha(double alpha)
{
	Alpha = alpha;
}



bool CMarkLogo::ProcessFrame(Mat & pFrame)
{
	Mat mask;

	LogoMask.copyTo(mask);

	Inpainter.inpaint(0, pFrame, mask);

	return true;
}
