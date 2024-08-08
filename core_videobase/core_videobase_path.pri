#区分不同的系统
path_sys = win
win32 {
path_sys = win
}

linux {
path_sys = linux
}

#Qt4套件不认识linux标记
unix:!macx {
path_sys = linux
}

macx {
path_sys = mac
}

android {
path_sys = android
}

#区分不同的位数 x86_64/amd64/arm64/arm64-v8a
path_bit = 32
contains(QT_ARCH, x.*64) {
path_bit = 64
} else:contains(QT_ARCH, a.*64) {
path_bit = 64
} else:contains(QT_ARCH, a.*64.*) {
path_bit = 64
}

#有些安卓套件(5.14/5.15)是4种一起/会导致切换到32位的套件的时候获取到的位数还是64位/可以在这里强制更改为32位
#path_bit = 32

#对应系统和位数的库目录/可以直接指定路径
path_lib = lib$$path_sys$$path_bit
#message($$path_lib)

win32 {
LIBS *= -luser32 -lopengl32 -lGLU32
}

unix:!macx:!android {
#下面几行用于编译期间设置可执行文件rpath路径(方便查找库)
#查看rpath命令 readelf -d xxx | grep 'RPATH'
#如果编译提示 -disable-new-dtags: unknown option 则把下面这行注释就行
DTAGS = ,-disable-new-dtags
QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN\'$$DTAGS"
QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/lib\'$$DTAGS"
QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/$$path_lib\'$$DTAGS"
}
