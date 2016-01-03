#ifndef CMARKTEXTBOX_H
#define CMARKTEXTBOX_H

#include <string>

using namespace std;

#include <xmarker/mark.h>

class CMarkTextBox : public CMark
{
public:
	bool ProcessFrame(std::queue<AVFrame *> & FrameBuffer);
	CMarkTextBox();
	~CMarkTextBox();
};

#endif
