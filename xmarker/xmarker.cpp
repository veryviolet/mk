#include <xmarker/xmarker.h>


CXMarker::CXMarker()
{
	BitRate_set = false;
	FrameBufferSize = 100;
	Transcoder.SetCallBack(XMarkerFrameProcessingCallback,this);
	Transcoder.SetTranscodeAudio(false);
	Transcoder.SetTranscodeVideo(true);

	pRGBFrame = NULL;
	forward_ctx = NULL;
	backward_ctx = NULL;
}


CXMarker::~CXMarker()
{
	if (pRGBFrame != NULL)
		av_frame_free(&pRGBFrame);

	if(forward_ctx != NULL)
		sws_freeContext(forward_ctx);

	if(backward_ctx != NULL)
		sws_freeContext(backward_ctx);
}

static void XMarkerFrameProcessingCallback(AVFrame * current, void * data)
{
	((CXMarker *) data)->ProcessFrame(current);
}

bool CXMarker::OpenSource()
{
	return Transcoder.OpenSource(SourceUrl.c_str());
}

bool CXMarker::OpenTarget()
{
	return Transcoder.OpenTarget(TargetUrl.c_str(),StreamName.c_str());
}

bool CXMarker::ReadConfiguration(const char * fname)
{
	try
	{
		dlib::config_reader ConfigReader(fname);


		if(!ConfigReader.is_block_defined("interfaces"))
		{
			printf("--> No interfaces in config file!\n");
			return false;
		}

		printf("--> Interface block found.\n");


		if (!ConfigReader.is_block_defined("marks"))
		{
			printf("--> No marks in config file!\n");
			return false;
		}

		printf("--> Marks block found.\n");

		if (!ConfigReader.block("interfaces").is_key_defined("source_url"))
		{
			printf("--> No source url in config file!\n");
			return false;
		}
		else
			SourceUrl = ConfigReader.block("interfaces")["source_url"];

		printf("--> Source url is: %s\n", SourceUrl.c_str());


		if (!ConfigReader.block("interfaces").is_key_defined("target_url"))
		{
			printf("--> No target url in config file!\n");
			return false;
		}
		else
			TargetUrl = ConfigReader.block("interfaces")["target_url"];

		printf("--> Target url is: %s\n", TargetUrl.c_str());

		if (!ConfigReader.block("interfaces").is_key_defined("stream_name"))
		{
			printf("--> No stream name in config file!\n");
			return false;
		}
		else
			StreamName = ConfigReader.block("interfaces")["stream_name"];


		std::vector<string> MarksNames;
		
		ConfigReader.block("marks").get_blocks(MarksNames);

		if (MarksNames.size() == 0)
		{
			printf("--> No marks in config file!\n");
			return false;
		}

		for (int i = 0; i < MarksNames.size(); i++)
		{
			string MarkType;

			if (!ConfigReader.block("marks").block(MarksNames[i]).is_key_defined("type"))
			{
				printf("--> No mark type for mark %s in config file!\n",MarksNames[i].c_str());
				return false;
			}
			else
				MarkType = ConfigReader.block("marks").block(MarksNames[i])["type"];


			CMark * m;

			if (MarkType == "textbox")
			{
				m = new CMarkTextBox();
			}
			else if (MarkType == "logo")
			{
				m = new CMarkLogo();

				if (!ConfigReader.block("marks").block(MarksNames[i]).is_key_defined("picture"))
				{
					printf("--> No picture for logo mark %s in config file!\n", MarksNames[i].c_str());
					return false;
				}
				else
					((CMarkLogo* ) m)->SetLogoImage(ConfigReader.block("marks").block(MarksNames[i])["picture"].c_str());

				if (!ConfigReader.block("marks").block(MarksNames[i]).is_key_defined("alpha"))
				{
					printf("--> No picture for logo mark %s in config file!\n", MarksNames[i].c_str());
					return false;
				}
				else
					((CMarkLogo*)m)->SetAlpha(atof(ConfigReader.block("marks").block(MarksNames[i])["alpha"].c_str()));
			}
			else
			{
				printf("--> Wrong mark type!\n");
				return false;
			}

			m->SetName(MarksNames[i]);

			Marks.push_back(m);
		}

	}
	catch (std::exception & ex)
	{
		printf("--> Error reading config file: %s\n",ex.what());
		return false;
	}

	return true;
}

bool CXMarker::Perform()
{
	if (!PrepareForProcessing())
		return false;

	return Transcoder.Process();
}

bool CXMarker::PrepareForProcessing()
{
	pRGBFrame = av_frame_alloc();

	if (pRGBFrame == NULL)
		return false;

	int bytes = avpicture_get_size(AV_PIX_FMT_BGR24, Transcoder.GetSourceWidth(), Transcoder.GetSourceHeight());
	uint8_t * buffer = (uint8_t *)av_malloc(bytes*sizeof(uint8_t));

	if (buffer == NULL)
		return false;

	avpicture_fill((AVPicture *)pRGBFrame, buffer, AV_PIX_FMT_BGR24,
		Transcoder.GetSourceWidth(), Transcoder.GetSourceHeight());

	forward_ctx = sws_getContext(Transcoder.GetSourceWidth(), Transcoder.GetSourceHeight(),
		Transcoder.GetSourcePixelFormat(),
		Transcoder.GetSourceWidth(), Transcoder.GetSourceHeight(), AV_PIX_FMT_BGR24, SWS_FAST_BILINEAR,
		NULL, NULL, NULL);

	if (forward_ctx == NULL)
		return false;

	backward_ctx = sws_getContext(Transcoder.GetSourceWidth(), Transcoder.GetSourceHeight(),
		AV_PIX_FMT_BGR24,
		Transcoder.GetSourceWidth(), Transcoder.GetSourceHeight(), Transcoder.GetSourcePixelFormat(), SWS_FAST_BILINEAR,
		NULL, NULL, NULL);

	if (forward_ctx == NULL)
		return false;


	return true;
}


void CXMarker::ProcessFrame(AVFrame * current)
{
	sws_scale(forward_ctx, current->data,
		current->linesize, 0,
		current->height,
		pRGBFrame->data, pRGBFrame->linesize);

	Mat mat(current->height, current->width, CV_8UC3, pRGBFrame->data[0], pRGBFrame->linesize[0]);

	for (int i = 0; i < Marks.size(); i++)
		Marks[i]->ProcessFrame(mat);

	sws_scale(backward_ctx, pRGBFrame->data,
		pRGBFrame->linesize, 0,
		current->height,
		current->data, current->linesize);

}



