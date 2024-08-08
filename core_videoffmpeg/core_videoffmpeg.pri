#将当前目录加入到头文件路径
INCLUDEPATH += $$PWD
include($$PWD/core_videoffmpeg_include.pri)

HEADERS += $$PWD/ffmpeginclude.h
HEADERS += $$PWD/ffmpegstruct.h

HEADERS += $$PWD/ffmpeghelper.h
SOURCES += $$PWD/ffmpeghelper.cpp

HEADERS += $$PWD/ffmpegutil.h
SOURCES += $$PWD/ffmpegutil.cpp

HEADERS += $$PWD/ffmpegfilter.h
SOURCES += $$PWD/ffmpegfilter.cpp

HEADERS += $$PWD/ffmpegdevice.h
SOURCES += $$PWD/ffmpegdevice.cpp

HEADERS += $$PWD/ffmpegsave.h
SOURCES += $$PWD/ffmpegsave.cpp

HEADERS += $$PWD/ffmpegsavehelper.h
SOURCES += $$PWD/ffmpegsavehelper.cpp

#HEADERS += $$PWD/other/ffmpegsaveaudio.h
#SOURCES += $$PWD/other/ffmpegsaveaudio.cpp

#HEADERS += $$PWD/other/ffmpegsavesimple.h
#SOURCES += $$PWD/other/ffmpegsavesimple.cpp

#当前组件功能较多可能被多个项目引入
#通过定义一个标识按照需要引入代码文件
contains(DEFINES, videoffmpeg) {
HEADERS += $$PWD/ffmpegsync.h
SOURCES += $$PWD/ffmpegsync.cpp

HEADERS += $$PWD/ffmpegthread.h
SOURCES += $$PWD/ffmpegthread.cpp

HEADERS += $$PWD/ffmpegthreadhelper.h
SOURCES += $$PWD/ffmpegthreadhelper.cpp

HEADERS += $$PWD/ffmpegwebsocket.h
SOURCES += $$PWD/ffmpegwebsocket.cpp
}

#拷贝字体文件到可执行文件同一目录/滤镜依赖这个字体文件
newPath($$DESTDIR)
copyToDestDir($$PWD/*.ttf, $$DESTDIR)
