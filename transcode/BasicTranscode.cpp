#include <transcode/BasicTranscode.h>


BasicTranscode::BasicTranscode()
{
	avcodec_register_all();
	av_register_all();
	avformat_network_init();

	callBack = &BasicTranscode::defaultBasicFrameProcessingCallback;

	SetTranscodeVideo(false);
	SetTranscodeAudio(false);
}


BasicTranscode::~BasicTranscode()
{

}

void BasicTranscode::SetCallBack(BasicFrameProcessingCallback cb, void * cb_data)
{
	callBack = cb;
	callback_data = cb_data;
}

void BasicTranscode::defaultBasicFrameProcessingCallback(AVFrame * current, void * data)
{

}


bool BasicTranscode::OpenSource(const char *url)
{
	int ret;
	unsigned int i;

	ifmt_ctx = NULL;
	if ((ret = avformat_open_input(&ifmt_ctx, url, NULL, NULL)) < 0) {
		return false;
	}

	if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0) {
		return false;
	}

	for (i = 0; i < ifmt_ctx->nb_streams; i++)
	{
		AVStream *stream;
		AVCodecContext *codec_ctx;
		stream = ifmt_ctx->streams[i];
		codec_ctx = stream->codec;
		/* Reencode video & audio and remux subtitles etc. */
		if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO
			|| codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
			/* Open decoder */
			ret = avcodec_open2(codec_ctx,
				avcodec_find_decoder(codec_ctx->codec_id), NULL);
			if (ret < 0) {
				return false;
			}
		}
	}

	return true;

}


bool BasicTranscode::OpenTarget(const char *url, const char * stream_name)
{
	AVStream *out_stream;
	AVStream *in_stream;
	AVCodecContext *dec_ctx, *enc_ctx;
	AVCodec *encoder;
	int ret;
	unsigned int i;


	ofmt_ctx = NULL;
	avformat_alloc_output_context2(&ofmt_ctx, NULL, "mpegts", url);
	if (!ofmt_ctx) {
		return false;
	}


	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
		out_stream = avformat_new_stream(ofmt_ctx, NULL);

		if (!out_stream) {
			return false;
		}

		in_stream = ifmt_ctx->streams[i];
		dec_ctx = in_stream->codec;
		enc_ctx = out_stream->codec;

		if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO
			|| dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
			/* in this example, we choose transcoding to same codec */
			encoder = avcodec_find_encoder(dec_ctx->codec_id);
			if (!encoder) {
				return false;
			}

			/* In this example, we transcode to same properties (picture size,
			* sample rate etc.). These properties can be changed for output
			* streams easily using filters */
			if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) 
			{
				if(stream_name != NULL)
					av_dict_set(&(out_stream->metadata), "streamName", (char *) stream_name, 0);
				
				SourceWidth = dec_ctx->width;
				SourceHeight = dec_ctx->height;
				SourcePixelFormat = dec_ctx->pix_fmt;

				enc_ctx->height = dec_ctx->height;
				enc_ctx->width = dec_ctx->width;
				enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;
				/* take first format from list of supported formats */
//				enc_ctx->pix_fmt = encoder->pix_fmts[0];
				enc_ctx->pix_fmt = dec_ctx->pix_fmt;
				/* video time_base can be set to whatever is handy and supported by encoder */
				enc_ctx->time_base = dec_ctx->time_base;

//				if (dec_ctx->codec_id == AV_CODEC_ID_H264)
//					av_opt_set(enc_ctx->priv_data, "preset", "fast", 0);
				enc_ctx->gop_size = 250;
				enc_ctx->keyint_min = 25;
				//enc_ctx->scenechange_threshold = 40;
				//enc_ctx->b_frame_strategy = 1;
				//enc_ctx->refs = 6;
				enc_ctx->max_b_frames = 0;
				enc_ctx->global_quality = 0;
				enc_ctx->qmin = 10;
				enc_ctx->qmax = 51;
				enc_ctx->i_quant_factor = 0.71;
				//enc_ctx->max_qdiff = 4;
				enc_ctx->profile = FF_PROFILE_H264_BASELINE;

				enc_ctx->bit_rate = dec_ctx->bit_rate;
				enc_ctx->bit_rate_tolerance = enc_ctx->bit_rate;
			}
			else {
				enc_ctx->sample_rate = dec_ctx->sample_rate;
				enc_ctx->channel_layout = dec_ctx->channel_layout;
				enc_ctx->channels = av_get_channel_layout_nb_channels(enc_ctx->channel_layout);
				/* take first format from list of supported formats */
				enc_ctx->sample_fmt = encoder->sample_fmts[0];
				AVRational r;
				r.num = 1;
				r.den = enc_ctx->sample_rate;
				enc_ctx->time_base = r;
			}

			/* Third parameter can be used to pass settings to encoder */
			ret = avcodec_open2(enc_ctx, encoder, NULL) ;
			if (ret < 0) {
				return false;
			}
		}
		else if (dec_ctx->codec_type == AVMEDIA_TYPE_UNKNOWN) {
			return false;
		}
		else {
			/* if this stream must be remuxed */
			ret = avcodec_copy_context(ofmt_ctx->streams[i]->codec,
				ifmt_ctx->streams[i]->codec);
			if (ret < 0) {
				return false;
			}
		}

		if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
			enc_ctx->flags |= AVFMT_GLOBALHEADER;


	}

	if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
		ret = avio_open(&ofmt_ctx->pb, url, AVIO_FLAG_WRITE);
		if (ret < 0) {
			return false;
		}
	}

	/* init muxer, write output file header */
	ret = avformat_write_header(ofmt_ctx, NULL);
	if (ret < 0) {
		return false;
	}

	return true;
}



bool BasicTranscode::EncodeAndWriteFrame(AVFrame *filt_frame, unsigned int stream_index, int *got_frame)
{
	int ret;
	int got_frame_local;
	AVPacket enc_pkt;
	int(*enc_func)(AVCodecContext *, AVPacket *, const AVFrame *, int *) =
		(ifmt_ctx->streams[stream_index]->codec->codec_type ==
		AVMEDIA_TYPE_VIDEO) ? avcodec_encode_video2 : avcodec_encode_audio2;

	if (!got_frame)
		got_frame = &got_frame_local;

	filt_frame->pts = av_frame_get_best_effort_timestamp(filt_frame);

	/* encode filtered frame */
	enc_pkt.data = NULL;
	enc_pkt.size = 0;
	av_init_packet(&enc_pkt);
	ret = enc_func(ofmt_ctx->streams[stream_index]->codec, &enc_pkt,
		filt_frame, got_frame);
//	av_frame_free(&filt_frame);
	if (ret < 0)
		return false;
	if (!(*got_frame))
		return true;

	/* prepare packet for muxing */
	enc_pkt.stream_index = stream_index;

	av_packet_rescale_ts(&enc_pkt,
		ofmt_ctx->streams[stream_index]->codec->time_base,
		ofmt_ctx->streams[stream_index]->time_base);

//	if (enc_pkt.dts != AV_NOPTS_VALUE)
//		enc_pkt.dts = av_rescale_q(enc_pkt.dts, ofmt_ctx->streams[stream_index]->codec->time_base, ofmt_ctx->streams[stream_index]->time_base);
	//if (ifmt_ctx->streams[stream_index]->codec->codec_type == AVMEDIA_TYPE_VIDEO && ifmt_ctx->streams[stream_index]->codec->codec_id == AV_CODEC_ID_H264) {
	//	AVBitStreamFilterContext* h264BitstreamFilterContext = av_bitstream_filter_init("h264_mp4toannexb");
	//	av_bitstream_filter_filter(h264BitstreamFilterContext, ofmt_ctx->streams[stream_index]->codec, NULL, &enc_pkt.data, &enc_pkt.size, enc_pkt.data, enc_pkt.size, 0);
	//}
	//else if (ifmt_ctx->streams[stream_index]->codec->codec_id == AV_CODEC_ID_AAC) {
	//	AVBitStreamFilterContext* aacBitstreamFilterContext = av_bitstream_filter_init("aac_adtstoasc");
	//	av_bitstream_filter_filter(aacBitstreamFilterContext, ofmt_ctx->streams[stream_index]->codec, NULL, &enc_pkt.data, &enc_pkt.size, enc_pkt.data, enc_pkt.size, 0);
	//}

	/* mux encoded frame */
	ret = av_interleaved_write_frame(ofmt_ctx, &enc_pkt);
	return (ret>=0);
}

bool BasicTranscode::FlushEncoder(unsigned int stream_index)
{
	int ret;
	int got_frame;

	if (!(ofmt_ctx->streams[stream_index]->codec->codec->capabilities &
		CODEC_CAP_DELAY))
		return true;

	while (1) {
		ret = EncodeAndWriteFrame(NULL, stream_index, &got_frame);
		if (ret < 0)
			return false;
		if (!got_frame)
			return true;
	}

	return true;
}


bool BasicTranscode::PostProcessAudioFrame(AVFrame ** pframe)
{
	av_frame_free(pframe);
	return true;
}

bool BasicTranscode::PostProcessVideoFrame(AVFrame ** pframe)
{
	av_frame_free(pframe);
	return true;
}


bool BasicTranscode::DecodePacket(AVFrame ** pFrame, AVPacket * pPacket, int * got_frame)
{
	int(*dec_func)(AVCodecContext *, AVFrame *, int *, const AVPacket *);

	*pFrame = av_frame_alloc();

	if (!*pFrame)
		return false;

	int stream_index = pPacket->stream_index;
	int type = ifmt_ctx->streams[pPacket->stream_index]->codec->codec_type;


	av_packet_rescale_ts(pPacket,
		ifmt_ctx->streams[stream_index]->time_base,
		ifmt_ctx->streams[stream_index]->codec->time_base);
	dec_func = (type == AVMEDIA_TYPE_VIDEO) ? avcodec_decode_video2 :
		avcodec_decode_audio4;

	int ret = dec_func(ifmt_ctx->streams[stream_index]->codec, *pFrame,
		got_frame, pPacket);

	if (ret < 0) {
		av_frame_free(pFrame);
		return false;
	}

	return true;
}


bool BasicTranscode::Process()
{
	int ret;
	AVFrame *frame = NULL;
	enum AVMediaType type;
	unsigned int stream_index;
	unsigned int i;
	int got_frame;

	Counter = 0;

	while (1) 
	{
		AVPacket * pPacket = new AVPacket();
		pPacket->data = 0;
		pPacket->size = 0;
		if ((ret = av_read_frame(ifmt_ctx, pPacket)) < 0)
		{
			delete pPacket;
			continue;
		}

		stream_index = pPacket->stream_index;
		type = ifmt_ctx->streams[pPacket->stream_index]->codec->codec_type;

		if (type == AVMEDIA_TYPE_VIDEO && bVideoTranscode)
		{
			DecodePacket(&frame, pPacket, &got_frame);

			if (got_frame)
			{
				Counter++;

				ProcessVideoFrame(frame);

				EncodeAndWriteFrame(frame, stream_index, NULL);

				PostProcessVideoFrame(&frame);
			}
			else
				av_frame_free(&frame);


		}
		else if (type == AVMEDIA_TYPE_AUDIO && bAudioTranscode)
		{
			DecodePacket(&frame, pPacket, &got_frame);

			if (got_frame)
			{
				ProcessAudioFrame(frame);

				EncodeAndWriteFrame(frame, stream_index, NULL);

				PostProcessAudioFrame(&frame);
			}
			else
				av_frame_free(&frame);

		}
		else 
			RemuxPacket(pPacket);

		delete pPacket;
	}

	/* flush filters and encoders */
	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
		/* flush filter */
		ret = EncodeAndWriteFrame(NULL, i, NULL);
		if (ret < 0) {
			goto end;
		}

		/* flush encoder */
		if (!FlushEncoder(i)) {
			goto end;
		}
	}

	av_write_trailer(ofmt_ctx);
end:
	//av_free_packet(&packet);
	av_frame_free(&frame);
	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
		avcodec_close(ifmt_ctx->streams[i]->codec);
		if (ofmt_ctx && ofmt_ctx->nb_streams > i && ofmt_ctx->streams[i] && ofmt_ctx->streams[i]->codec)
			avcodec_close(ofmt_ctx->streams[i]->codec);
	}
	avformat_close_input(&ifmt_ctx);
	if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
		avio_closep(&ofmt_ctx->pb);
	avformat_free_context(ofmt_ctx);

	return ret >= 0;
}

void BasicTranscode::RemuxPacket(AVPacket * packet)
{
	av_packet_rescale_ts(packet,
		ifmt_ctx->streams[packet->stream_index]->time_base,
		ofmt_ctx->streams[packet->stream_index]->time_base);

	av_interleaved_write_frame(ofmt_ctx, packet);
}

int BasicTranscode::GetSourceWidth()
{
	return SourceWidth;
}

int BasicTranscode::GetSourceHeight()
{
	return SourceHeight;
}

AVPixelFormat BasicTranscode::GetSourcePixelFormat()
{
	return SourcePixelFormat;
}


