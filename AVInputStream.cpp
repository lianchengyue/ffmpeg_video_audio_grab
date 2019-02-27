#include "AVInputStream.h"

#include <unistd.h>

/* read a key without blocking */
static int read_key(void)
{
    unsigned char ch;

    int n = 1;
    struct timeval tv;
    fd_set rfds;

    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    n = select(1, &rfds, NULL, NULL, &tv);
    if (n > 0) {
        n = read(0, &ch, 1);
        if (n == 1)
            return ch;

        return n;
    }
    return -1;
}
CAVInputStream::CAVInputStream(void)
{
	m_hCapVideoThread = NULL;
	m_hCapAudioThread = NULL;
	m_exit_thread = false;

	m_pVidFmtCtx = NULL;
	m_pAudFmtCtx = NULL;
	m_pInputFormat = NULL;
    m_pAudioInputFormat = NULL;

	dec_pkt = NULL;

	m_pVideoCBFunc = NULL;
	m_pAudioCBFunc = NULL;

	m_videoindex = -1;
	m_audioindex = -1;

	m_start_time = 0;

	//avcodec_register_all();
 //   av_register_all();
	//avdevice_register_all();  
}

CAVInputStream::~CAVInputStream(void)
{
    printf("Desctruction Input!\n");
	CloseInputStream();
}

	
void  CAVInputStream::SetVideoCaptureCB(VideoCaptureCB pFuncCB)
{
    m_pVideoCBFunc = pFuncCB;
}
	
void  CAVInputStream::SetAudioCaptureCB(AudioCaptureCB pFuncCB)
{
    m_pAudioCBFunc = pFuncCB;
}
	
void  CAVInputStream::SetVideoCaptureDevice(string device_name)
{
    m_video_device = device_name;
}

void  CAVInputStream::SetAudioCaptureDevice(string device_name)
{
	m_audio_device = device_name;
}


bool  CAVInputStream::OpenInputStream()
{
	if(m_video_device.empty() && m_audio_device.empty())
	{
        printf("you have not set any capture device \n");
		return false;
	}


	int i;

    ///视频...
    m_pInputFormat = av_find_input_format("video4linux2"); //v4l2
    assert(m_pInputFormat != NULL);

    // Set device params
    AVDictionary *device_param = 0;
	//if not setting rtbufsize, error messages will be shown in cmd, but you can still watch or record the stream correctly in most time
	//setting rtbufsize will erase those error messages, however, larger rtbufsize will bring latency
    //av_dict_set(&device_param, "rtbufsize", "10M", 0);

	if(!m_video_device.empty())
	{
		int res = 0;

        string device_name = m_video_device;

		 //Set own video device's name
        if ((res = avformat_open_input(&m_pVidFmtCtx, device_name.c_str(), m_pInputFormat, &device_param)) != 0)
		{
            printf("Couldn't open input video stream.（无法打开输入流）\n");
			return false;
		}
		//input video initialize
		if (avformat_find_stream_info(m_pVidFmtCtx, NULL) < 0)
		{
            printf("Couldn't find video stream information.（无法获取流信息）\n");
			return false;
		}
		m_videoindex = -1;
		for (i = 0; i < m_pVidFmtCtx->nb_streams; i++)
		{
			if (m_pVidFmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				m_videoindex = i;
				break;
			}
		}

		if (m_videoindex == -1)
		{
            printf("Couldn't find a video stream.（没有找到视频流）\n");
			return false;
		}

		if (avcodec_open2(m_pVidFmtCtx->streams[m_videoindex]->codec, avcodec_find_decoder(m_pVidFmtCtx->streams[m_videoindex]->codec->codec_id), NULL) < 0)
		{
            printf("Could not open video codec.（无法打开解码器）\n");
			return false;
		}

        /* print Video device information*/
        av_dump_format(m_pVidFmtCtx, 0, "video4linux2", 0);
	}

    ///音频...
    //////////////////////////////////////////////////////////
    // input/output: alsa, decklink, fbdev, oss, pulse, sndio, v4l2
    // input: avfoundation, bktr, dshow, dv1394, gdigrab, iec61883, jack, lavfi,openal, vfwcap, x11grab, x11grab_xcb
    //output: caca, opengl, qtkit, sdl, xv
    //x11grab: desktop
    //m_pAudioInputFormat = av_find_input_format("oss"); //oss
    //TQQ
    m_pAudioInputFormat = av_find_input_format("alsa"); //alsa
    assert(m_pAudioInputFormat != NULL);
    if(m_pAudioInputFormat == NULL)
    {
        printf("did not find this audio input devices\n");
    }

	if(!m_audio_device.empty())
	{
        string device_name = m_audio_device;

		//Set own audio device's name
        if (avformat_open_input(&m_pAudFmtCtx, device_name.c_str(), m_pAudioInputFormat, &device_param) != 0){

            printf("Couldn't open input audio stream.（无法打开输入流）\n");
			return false;
		}

		//input audio initialize
		if (avformat_find_stream_info(m_pAudFmtCtx, NULL) < 0)
		{
            printf("Couldn't find audio stream information.（无法获取流信息）\n");
			return false;
		}
		m_audioindex = -1;
		for (i = 0; i < m_pAudFmtCtx->nb_streams; i++)
		{
			if (m_pAudFmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
			{
				m_audioindex = i;
				break;
			}
		}
		if (m_audioindex == -1)
		{
            printf("Couldn't find a audio stream.（没有找到音频流）\n");
			return false;
		}
       	        ///Caution, m_pAudFmtCtx->streams[m_audioindex]->codec->codec_id =14, AV_CODEC_ID_RAWVIDEO
		if (avcodec_open2(m_pAudFmtCtx->streams[m_audioindex]->codec, avcodec_find_decoder(m_pAudFmtCtx->streams[m_audioindex]->codec->codec_id), NULL) < 0)
		{
            printf("Could not open audio codec.（无法打开解码器）\n");
			return false;
		}

        /* print Video device information*/
        av_dump_format(m_pAudFmtCtx, 0, "default", 0);
	}

	return true;
}

bool  CAVInputStream::StartCapture()
{
	if (m_videoindex == -1 && m_audioindex == -1)
	{
        printf("错误：你没有打开设备 \n");
		return false;
	}

    m_start_time = av_gettime();

	m_exit_thread = false;

	if(!m_video_device.empty())
	{
        int rc = pthread_create(&m_hCapVideoThread, NULL, CaptureVideoThreadFunc, (void *)this);
	}

	if(!m_audio_device.empty())
	{
        int rc = pthread_create(&m_hCapAudioThread, NULL, CaptureAudioThreadFunc, (void *)this);
    }

    if(!m_video_device.empty())
    {
        pthread_join(m_hCapVideoThread,NULL);
    }

    if(!m_audio_device.empty())
    {
        pthread_join(m_hCapAudioThread,NULL);
    }

	return true;
}

void CAVInputStream::CloseInputStream()
{
	m_exit_thread = true;

    //关闭线程
    if(m_hCapVideoThread)
    {
        m_hCapVideoThread = NULL;// 0
    }

    if(m_hCapAudioThread)
    {
        m_hCapAudioThread = NULL;
    }

	 //关闭输入流  
    if (m_pVidFmtCtx != NULL)  
    {  
        avformat_close_input(&m_pVidFmtCtx);    
    }  
    if (m_pAudFmtCtx != NULL)  
    {  
        avformat_close_input(&m_pAudFmtCtx);  
    }  

	if(m_pVidFmtCtx)
	   avformat_free_context(m_pVidFmtCtx);
	if(m_pAudFmtCtx)
	   avformat_free_context(m_pAudFmtCtx);

    m_pVidFmtCtx = NULL;
    m_pAudFmtCtx = NULL;
    m_pInputFormat = NULL;

	m_videoindex = -1;
	m_audioindex = -1;
}

void* CAVInputStream::CaptureVideoThreadFunc(void* lParam)
{
	CAVInputStream * pThis = (CAVInputStream*)lParam;

    printf("CaptureVideoThread\n");
	pThis->ReadVideoPackets();
}

int CAVInputStream::ReadVideoPackets()
{
	if(dec_pkt == NULL)
	{
		////prepare before decode and encode
		dec_pkt = (AVPacket *)av_malloc(sizeof(AVPacket));
	}

	int encode_video = 1;
	int ret;

	//start decode and encode
	while (encode_video)
	{
		if (m_exit_thread)
			break;

        /* if 'Enter' pressed, exits */
        //printf("read_key()=%d\n\n", read_key());
        if (10 == read_key())
        {
            pthread_exit((void*)0);
        }

		AVFrame * pframe = NULL;
		if ((ret = av_read_frame(m_pVidFmtCtx, dec_pkt)) >= 0)
		{
			pframe = av_frame_alloc();
			if (!pframe) 
			{
				ret = AVERROR(ENOMEM);
				return ret;
			}
			int dec_got_frame = 0;
			ret = avcodec_decode_video2(m_pVidFmtCtx->streams[dec_pkt->stream_index]->codec, pframe, &dec_got_frame, dec_pkt);
			if (ret < 0) 
			{
				av_frame_free(&pframe);
				av_log(NULL, AV_LOG_ERROR, "Decoding failed\n");
				break;
			}
			if (dec_got_frame)
			{
                if(m_pVideoCBFunc)
				{
                    pthread_mutex_lock(&mutex);
                    //打时间戳,modified by flq,tqq
                    m_pVideoCBFunc(m_pVidFmtCtx->streams[dec_pkt->stream_index], m_pVidFmtCtx->streams[m_videoindex]->codec->pix_fmt, pframe, av_gettime() - m_start_time);
                    //解决时间戳不同步问题
                    //m_pVideoCBFunc(m_pVidFmtCtx->streams[dec_pkt->stream_index], m_pVidFmtCtx->streams[m_videoindex]->codec->pix_fmt, pframe, av_gettime()/* - m_start_time*/);
                    pthread_mutex_unlock(&mutex);
                }

				av_frame_free(&pframe);
			}
			else 
			{
				av_frame_free(&pframe);
			}

			av_free_packet(dec_pkt);
		}
		else
		{
			if (ret == AVERROR_EOF)
				encode_video = 0;
			else
			{
                printf("Could not read video frame\n");
				break;
			}
		}
	}

	return 0;
}

void* CAVInputStream::CaptureAudioThreadFunc(void* lParam)
{
	CAVInputStream * pThis = (CAVInputStream*)lParam;

    printf("CaptureAudioThread\n");
	pThis->ReadAudioPackets();
}

//ReadAudioPackets + ReadVideoPackets
int CAVInputStream::ReadAudioPackets()
{
	//audio trancoding here
    int ret;

	int encode_audio = 1;
    int dec_got_frame_a = 0;

	//start decode and encode
	while (encode_audio)
	{
		if (m_exit_thread)
        {
		   break;
        }

        /* if 'Enter' pressed, exits */
        //printf("read_key()=%d\n\n", read_key());
        if (10 == read_key())
        {
            pthread_exit((void*)0);
        }

		/**
		* Decode one frame worth of audio samples, convert it to the
		* output sample format and put it into the FIFO buffer.
		*/
		AVFrame *input_frame = av_frame_alloc();
		if (!input_frame)
		{
			ret = AVERROR(ENOMEM);
			return ret;
		}			

		/** Decode one frame worth of audio samples. */
		/** Packet used for temporary storage. */
		AVPacket input_packet;
		av_init_packet(&input_packet);
		input_packet.data = NULL;
		input_packet.size = 0;

		/** Read one audio frame from the input file into a temporary packet. */
		if ((ret = av_read_frame(m_pAudFmtCtx, &input_packet)) < 0) 
		{
			/** If we are at the end of the file, flush the decoder below. */
			if (ret == AVERROR_EOF)
			{
				encode_audio = 0;
			}
			else
			{
                //printf("Could not read audio frame\n");
//				return ret;
                //added by flq
                continue;
                //added end
			}					
		}
        else
        {
            //打印音频输出
#ifdef DEBUG
            //printf("size=%d\n%s\n\n\n", strlen((char*)input_packet.data), input_packet.data);
#endif
        }

		/**
		* Decode the audio frame stored in the temporary packet.
		* The input audio stream decoder is used to do this.
		* If we are at the end of the file, pass an empty packet to the decoder
		* to flush it.
		*/
#ifdef DEBUG
        //printf("frame comes\n");
#endif
		if ((ret = avcodec_decode_audio4(m_pAudFmtCtx->streams[m_audioindex]->codec, input_frame, &dec_got_frame_a, &input_packet)) < 0)
		{
            printf("Could not decode audio frame\n");
			return ret;
		}
		av_packet_unref(&input_packet);
		/** If there is decoded data, convert and store it */
		if (dec_got_frame_a) 
		{
		    if(m_pAudioCBFunc)
			{
                pthread_mutex_lock(&mutex);
                //param3:打时间戳 //tqq
				m_pAudioCBFunc(m_pAudFmtCtx->streams[m_audioindex], input_frame, av_gettime() - m_start_time);
                pthread_mutex_unlock(&mutex);
            }
		}

		av_frame_free(&input_frame);
		

	}//while

	return 0;
}

	
bool CAVInputStream::GetVideoInputInfo(int & width, int & height, int & frame_rate, AVPixelFormat & pixFmt)
{
	if(m_videoindex != -1)
	{
		width  =  m_pVidFmtCtx->streams[m_videoindex]->codec->width;
		height =  m_pVidFmtCtx->streams[m_videoindex]->codec->height;
		
		AVStream *stream = m_pVidFmtCtx->streams[m_videoindex];  
  
		pixFmt = stream->codec->pix_fmt;

                //帧率
		if(stream->r_frame_rate.den > 0)
		{
		  frame_rate = stream->r_frame_rate.num/stream->r_frame_rate.den;
		}
		else if(stream->codec->framerate.den > 0)
		{
		  frame_rate = stream->codec->framerate.num/stream->codec->framerate.den;
		}

		return true;
	}

	return false;
}

bool  CAVInputStream::GetAudioInputInfo(AVSampleFormat & sample_fmt, int & sample_rate, int & channels)
{
	if(m_audioindex != -1)
	{
		sample_fmt = m_pAudFmtCtx->streams[m_audioindex]->codec->sample_fmt;
		sample_rate = m_pAudFmtCtx->streams[m_audioindex]->codec->sample_rate;
		channels = m_pAudFmtCtx->streams[m_audioindex]->codec->channels;

		return true;
	}

	return false;
}
