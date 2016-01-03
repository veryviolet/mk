#ifndef XMARKER_BASICTRANSCODE_H
#define XMARKER_BASICTRANSCODE_H

extern "C" {
#define __STDC_CONSTANT_MACROS
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
};

#include <deque>

using namespace std;


typedef struct FilteringContext {
    AVFilterContext *buffersink_ctx;
    AVFilterContext *buffersrc_ctx;
    AVFilterGraph *filter_graph;
} FilteringContext;


typedef void(*BasicFrameProcessingCallback)(AVFrame * current, void * data);

class BasicTranscode
{
protected:
    AVFormatContext *ifmt_ctx;
    AVFormatContext *ofmt_ctx;
    FilteringContext *filter_ctx;
    BasicFrameProcessingCallback callBack;
    void * callback_data;

	bool bAudioTranscode;
	bool bVideoTranscode;

	int SourceWidth;
	int SourceHeight;
	AVPixelFormat SourcePixelFormat;

	int64_t Counter;

public:
	BasicTranscode();
    ~BasicTranscode();
	void SetTranscodeAudio(bool ta) { bAudioTranscode = ta; };
	void SetTranscodeVideo(bool tv) { bVideoTranscode = tv; };
    bool OpenSource(const char *url);
    bool OpenTarget(const char *url, const char *stream_name);
    virtual bool Process();
	virtual bool ProcessAudioFrame(AVFrame * frame) = 0;
	virtual bool ProcessVideoFrame(AVFrame * frame) = 0;
	virtual bool PostProcessAudioFrame(AVFrame ** pframe);
	virtual bool PostProcessVideoFrame(AVFrame ** pframe);
	bool DecodePacket(AVFrame ** pFrame, AVPacket * pPacket, int * got_frame);
    void SetCallBack(BasicFrameProcessingCallback cb, void * cb_data);
	int GetSourceWidth();
	int GetSourceHeight();
	AVPixelFormat GetSourcePixelFormat();
protected:
	static void defaultBasicFrameProcessingCallback(AVFrame * current, void * data);
private:
    bool EncodeAndWriteFrame(AVFrame *filt_frame, unsigned int stream_index, int *got_frame);
    bool FlushEncoder(unsigned int stream_index);
	void RemuxPacket(AVPacket * packet);
};


#endif
