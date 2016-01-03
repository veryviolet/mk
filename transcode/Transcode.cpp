#include <transcode/Transcode.h>


Transcode::Transcode()
{
	avcodec_register_all();
	av_register_all();
	avformat_network_init();

	callBack = &BasicTranscode::defaultBasicFrameProcessingCallback;

}


Transcode::~Transcode()
{

}

bool Transcode::ProcessAudioFrame(AVFrame * frame)
{
	return true;
}

bool Transcode::ProcessVideoFrame(AVFrame * frame)
{
	callBack(frame, callback_data);
	return true;
}

