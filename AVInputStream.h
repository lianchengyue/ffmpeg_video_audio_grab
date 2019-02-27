#ifndef AVINPUTSTREAM_H
#define AVINPUTSTREAM_H

#include <string>
#include <assert.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libavutil/audio_fifo.h>
#include <libswresample/swresample.h>

#include <libavutil/time.h> ////av_gettime()
}

using namespace std;

//typedef double (*VideoCaptureCB)(); //OK
typedef int (*VideoCaptureCB)(AVStream * input_st, enum PixelFormat pix_fmt, AVFrame *pframe, int64_t lTimeStamp);
typedef int (*AudioCaptureCB)(AVStream *input_st, AVFrame *pframe, int64_t lTimeStamp);

//typedef LRESULT (CALLBACK* VideoCaptureCB)(AVStream * input_st, enum PixelFormat pix_fmt, AVFrame *pframe, INT64 lTimeStamp);
//typedef LRESULT (CALLBACK* AudioCaptureCB)(AVStream * input_st, AVFrame *pframe, INT64 lTimeStamp);


class CAVInputStream
{
public:
	CAVInputStream(void);
	~CAVInputStream(void);

public:
	void  SetVideoCaptureDevice(string device_name);
	void  SetAudioCaptureDevice(string device_name);

	bool  OpenInputStream();
	void  CloseInputStream();

	bool  StartCapture();

	void  SetVideoCaptureCB(VideoCaptureCB pFuncCB);
	void  SetAudioCaptureCB(AudioCaptureCB pFuncCB);

	bool  GetVideoInputInfo(int & width, int & height, int & framerate, AVPixelFormat & pixFmt);
	bool  GetAudioInputInfo(AVSampleFormat & sample_fmt, int & sample_rate, int & channels);

protected:
//    static /*unsigned long*/void  CaptureVideoThreadFunc(void* lParam);
//    static /*unsigned long*/void  CaptureAudioThreadFunc(void* lParam);
    static void  *CaptureVideoThreadFunc(void* lParam);
    static void  *CaptureAudioThreadFunc(void* lParam);

	int  ReadVideoPackets();
	int  ReadAudioPackets();

//protected:
public:
	string  m_video_device;
    string b;
	string  m_audio_device;

	int     m_videoindex;
    int     m_audioindex;

    AVFormatContext *m_pVidFmtCtx;
    AVFormatContext *m_pAudFmtCtx;
    AVInputFormat  *m_pInputFormat;
    AVInputFormat  *m_pAudioInputFormat;


    AVPacket *dec_pkt;

    pthread_t  m_hCapVideoThread, m_hCapAudioThread; //线程句柄
	bool   m_exit_thread; //退出线程的标志变量

	VideoCaptureCB  m_pVideoCBFunc; //视频数据回调函数指针
	AudioCaptureCB  m_pAudioCBFunc; //音频数据回调函数指针

///	CCritSec     m_WriteLock;
    pthread_mutex_t mutex;
/// pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

	int64_t     m_start_time; //采集的起点时间
};

#endif //AVINPUTSTREAM_H
