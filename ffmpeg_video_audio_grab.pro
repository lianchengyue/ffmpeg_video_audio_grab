QT += core
QT -= gui

CONFIG += c++11

TARGET = ffmpeg_video_audio_grab
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

#DEFINES+=VIDEO_RESCALE

HEADERS += \
    AVInputStream.h \
    AVOutputStream.h \
    MainLoop.h

SOURCES += \
    AVInputStream.cpp \
    AVOutputStream.cpp \
    MainLoop.cpp


INCLUDEPATH += \
#opencv
/usr/local/opencv2.4.13/include \
/usr/local/opencv2.4.13/include/opencv \
/usr/local/opencv2.4.13/include/opencv2 \
#h264
#ffmpeg
#/usr/local/include\libavcodec \
#/usr/local/include\libavdevice \
#/usr/local/include\libavfilter \
#/usr/local/include\libavformat \
#/usr/local/include\libavutil \
#/usr/local/include\libswresample \
#/usr/local/include\libswscale \
#
#/usr/include/x86_64-linux-gnu/libavcodec \
#/usr/include/x86_64-linux-gnu/libavdevice \
#/usr/include/x86_64-linux-gnu/libavfilter \
#/usr/include/x86_64-linux-gnu/libavformat \
#/usr/include/x86_64-linux-gnu/libavutil \
#/usr/include/x86_64-linux-gnu/libswresample \
#/usr/include/x86_64-linux-gnu/libswscale \

LIBS +=  \
#opencv
/usr/local/opencv2.4.13/lib/libopencv_*.so \
#h264
/usr/lib/x86_64-linux-gnu/libx264.so \
#ffmpeg
#/usr/local/lib/libavformat.a \
#/usr/local/lib/libavdevice.a \
#/usr/local/lib/libavcodec.a \
#/usr/local/lib/libavfilter.a \
#/usr/local/lib/libavutil.a \
#/usr/local/lib/libswscale.a \
#/usr/local/lib/libswresample.a \
-lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswscale -lswresample \
#-lx264
-lmp3lame -lz -lbz2 -llzma \
-lva-x11 -lva \
-lxcb-shm -lxcb-xfixes -lxcb-render -lxcb-shape -lxcb -lXau -lXdmcp -lasound -ldl -lX11 -lXext \
-lpthread -llzma -lrt -lm \
