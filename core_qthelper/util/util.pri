HEADERS += \
    $$PWD/customstyle.h \
    $$PWD/globalconfig.h \
    $$PWD/globalhead.h \
    $$PWD/globalstyle.h \
    $$PWD/iconhelper.h \
    $$PWD/nativeform.h \
    $$PWD/singleton.h

SOURCES += \
    $$PWD/customstyle.cpp \
    $$PWD/globalconfig.cpp \
    $$PWD/globalstyle.cpp \
    $$PWD/iconhelper.cpp \
    $$PWD/nativeform.cpp

wasm {
HEADERS += $$PWD/wasmhelper.h
SOURCES += $$PWD/wasmhelper.cpp
}
