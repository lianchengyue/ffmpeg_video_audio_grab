//pts: （int64_t）显示时间，结合AVStream->time_base转换成时间戳
//dts: （int64_t）解码时间，结合AVStream->time_base转换成时间戳

#ifndef MAINLOOP_H
#define MAINLOOP_H

#include "AVInputStream.h"
#include "AVOutputStream.h"
#include <sys/time.h>

#include <map>

#define AUDIO_INPUT_DEVICE    "hw:0,0"        //hw:0,0
#define VIDEO_INPUT_DEVICE    "/dev/video0"   ///dev/video0

extern "C"
{
#include <libavdevice/avdevice.h>
}

using namespace std;


class MainLoop
{

public:
    MainLoop();
    virtual ~MainLoop();

public:
    void   CalculateFPS();
    bool   IsPreview() { return m_bPreview; }

    int OnStartStream();
    int OnStopStream();

    void OnDestroy();
protected:
    long              m_nChannelNum; //通道号

    uint64_t              m_frmCount;
    int              m_nFPS;
    bool              m_bPreview;
    char* m_szFilePath;

    timeval p_start;
    timeval p_end;

public:
    CAVInputStream    m_InputStream;
    CAVOutputStream   m_OutputStream;

};


#endif // MAINLOOP_H
