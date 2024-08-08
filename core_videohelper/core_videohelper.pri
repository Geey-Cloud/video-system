#将当前目录加入到头文件路径
INCLUDEPATH += $$PWD

HEADERS += $$PWD/osdgraph.h
SOURCES += $$PWD/osdgraph.cpp

HEADERS += $$PWD/urlutil.h
SOURCES += $$PWD/urlutil.cpp

HEADERS += $$PWD/urlhelper.h
SOURCES += $$PWD/urlhelper.cpp

HEADERS += $$PWD/filterhelper.h
SOURCES += $$PWD/filterhelper.cpp

HEADERS += $$PWD/deviceinfohelper.h
SOURCES += $$PWD/deviceinfohelper.cpp

#没有引入过则引入这个类
!contains(DEFINES, imagelabel) {
DEFINES *= imagelabel
HEADERS += $$PWD/imagelabel.h
SOURCES += $$PWD/imagelabel.cpp
}

#没有引入音频模块则引用一个空类
!contains(DEFINES, audiox) {
HEADERS += $$PWD/audioplayer.h
SOURCES += $$PWD/audioplayer.cpp
}
