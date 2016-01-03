#ifndef CXMARKER_H
#define CXMARKER_H

#include <transcode/Transcode.h>

extern "C" {
#define __STDC_CONSTANT_MACROS
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
#include <libavutil/pixfmt.h>
};

#include <dlib/config_reader.h>

#include <xmarker/mark.h>
#include <xmarker/marktextbox.h>
#include <xmarker/marklogo.h>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect.hpp>

#include <deque>
#include <string>
#include <vector>
#include <string.h>
#include <math.h>

using namespace std;

static void XMarkerFrameProcessingCallback(AVFrame * current, void * data);

class CXMarker
{
	string SourceUrl;
	string TargetUrl;

	string StreamName;

	std::vector<CMark *> Marks;

	int BitRate;
	bool BitRate_set;

	int FrameBufferSize;
	Transcode Transcoder;

	struct SwsContext *forward_ctx;
	struct SwsContext *backward_ctx;

	AVFrame * pRGBFrame;


public:
	CXMarker();
	~CXMarker();
	bool ReadConfiguration(const char * fname);
	void ProcessFrame(AVFrame * current);
	bool OpenSource();
	bool OpenTarget();
	bool Perform();
	bool PrepareForProcessing();
};

#endif
