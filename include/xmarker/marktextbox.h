#ifndef CMARKTEXTBOX_H
#define CMARKTEXTBOX_H

#include <string>

using namespace std;

#include <xmarker/mark.h>

#include <text/erfilter.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/videostab.hpp>
#include <opencv2/videostab/inpainting.hpp>

using namespace cv::videostab;
using namespace cv;
using namespace cv::text;

#define THRESH 20
#define DIFF_THRESH 50

class CMarkTextBox : public CMark
{
	unsigned int BackColor;
	unsigned int ForeColor;
	unsigned int ApproximateWidth;
	unsigned int ApproximateHeight;

	int MarkWidth;
	int MarkHeight;
	int DeltaSize;
	int DeltaInpaint;


	Mat FoundMark;
	Rect FoundRect;
	bool Found;

	double NormDiff;

	Mat Mask;

	ColorAverageInpainter Inpainter;

	int counter;
	vector<Mat> channels;
	vector<vector<ERStat> > regions;
public:
//	bool SetTemplate(const char * fname);
	void SetBackColor(unsigned int bc);
	void SetForeColor(unsigned int fc);
	void SetApproximateWidth(unsigned int aw);
	void SetApproximateHeight(unsigned int ah);
	bool ProcessFrame(Mat & pFrame);
	CMarkTextBox();
	~CMarkTextBox();

private:
	void Inpaint(Mat & pFrame, Rect R);
	void groups_draw(Mat &src, vector<Rect> &groups);
};

#endif
