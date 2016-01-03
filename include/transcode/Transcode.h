#ifndef XMARKER_TRANSCODE_H
#define XMARKER_TRANSCODE_H

#include <transcode/BasicTranscode.h>

class Transcode : public BasicTranscode
{
public:
    Transcode();
    ~Transcode();
	bool ProcessAudioFrame(AVFrame * frame);
	bool ProcessVideoFrame(AVFrame * frame);
};


#endif
