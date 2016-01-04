#include <xmarker/marktextbox.h>

CMarkTextBox::CMarkTextBox()
{
	Inpainter.setRadius(10);

	MarkWidth = 128;
	MarkHeight = 22;
	DeltaSize = 50;
	DeltaInpaint = 20;

	NormDiff = 1;

	regions.resize(2);

	Found = false;

}



CMarkTextBox::~CMarkTextBox()
{
}

void CMarkTextBox::SetBackColor(unsigned int bc)
{
	BackColor = bc;
}

void CMarkTextBox::SetForeColor(unsigned int fc)
{
	ForeColor = fc;
}

void CMarkTextBox::SetApproximateWidth(unsigned int aw)
{
	ApproximateWidth = aw;
}

void CMarkTextBox::SetApproximateHeight(unsigned int ah)
{
	ApproximateHeight = ah;
}

void CMarkTextBox::Inpaint(Mat & pFrame, Rect R)
{
	try
	{
		Rect toinpaint(max(R.x - DeltaInpaint, 0), max(R.y - DeltaInpaint, 0),
			min(R.width + 2 * DeltaInpaint, pFrame.cols - R.x + DeltaInpaint),
			min(R.height + 2 * DeltaInpaint, pFrame.rows - R.y + DeltaInpaint));

		Mask.setTo(255);
		Mask(toinpaint).setTo(0);
		Inpainter.inpaint(1, pFrame, Mask);
	}
	catch (cv::Exception e)
	{
		printf(e.msg.c_str());
	}

}


bool CMarkTextBox::ProcessFrame(Mat & pFrame)
{
	return true;
	if (Found)
	{
		Mat curMark = pFrame(FoundRect);

		double nrm = norm(curMark, FoundMark, TM_SQDIFF_NORMED);

		if (nrm < NormDiff)
		{
//			rectangle(pFrame, FoundRect, Scalar(0, 255, 0), 4);
			Inpaint(pFrame, FoundRect);
			curMark.copyTo(FoundMark);
			return true;
		}
		else
		{
//			imwrite("m_prev.jpg", FoundMark);
//			imwrite("m_curr.jpg", curMark);

			Found = false;
		}
	}

	Mat grey;

	if (Mask.data == NULL)
		Mask = Mat(pFrame.rows, pFrame.cols, CV_8U);

	cvtColor(pFrame, grey, COLOR_BGR2GRAY);

	channels.clear();
	channels.push_back(grey);
	channels.push_back(255 - grey);


	regions[0].clear();
	regions[1].clear();

	vector<vector<Point> > contours;
	vector<Rect> bboxes;
	Ptr<MSER> mser = MSER::create(31, 5 * 10, 12 * 24 , 1, 0.7);
	mser->detectRegions(grey, contours, bboxes);

	if (contours.size() > 0)
		MSERsToERStats(grey, contours, regions);

	vector< vector<Vec2i> > nm_region_groups;
	vector<Rect> nm_boxes;

	erGrouping(pFrame, channels, regions, nm_region_groups, nm_boxes, ERGROUPING_ORIENTATION_HORIZ);

	for (int i = 0; i < nm_boxes.size(); i++)
	{
		if ((abs(nm_boxes[i].width - MarkWidth) < DeltaSize) && (abs(nm_boxes[i].width - MarkWidth) < DeltaSize))
		{
			FoundRect = nm_boxes[i];
			pFrame(FoundRect).copyTo(FoundMark);
			Found = true;

//			rectangle(pFrame, FoundRect, Scalar(0, 255, 0), 4);

			Inpaint(pFrame, nm_boxes[i]);

		}
	}

	return true;
}

void CMarkTextBox::groups_draw(Mat &src, vector<Rect> &groups)
{
	for (int i = (int)groups.size() - 1; i >= 0; i--)
	{
		if (src.type() == CV_8UC3)
			rectangle(src, groups.at(i).tl(), groups.at(i).br(), Scalar(0, 255, 255), 3, 8);
		else
			rectangle(src, groups.at(i).tl(), groups.at(i).br(), Scalar(255), 3, 8);
	}
}
