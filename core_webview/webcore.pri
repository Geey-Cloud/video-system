#默认假定采用webkit,因为Qt4没有qtHaveModule函数而且默认采用的是webkit
#如果在windows上没有浏览器模块则默认采用miniblink
#Qt5+mingw webkit 下载地址 https://github.com/qtwebkit/qtwebkit/releases
DEFINES += webkit

#判断当前qt版本号
message($$QT_ARCH : $$QT_VERSION -> $$QT_MAJOR_VERSION . $$QT_MINOR_VERSION)

#5.0以上版本
!lessThan(QT_MAJOR_VERSION, 5) {
#如果没有安装webkit模块
!qtHaveModule(webkit) {
DEFINES -= webkit
}
#如果安装了webenginewidgets模块
qtHaveModule(webenginewidgets) {
DEFINES += webengine
}}

#如果采用IE浏览器则改成 DEFINES += webie
#如果采用miniblink改成 DEFINES += webminiblink  只支持win
#在win上也可以手动强制使用 miniblink 就算存在 webkit 或者 webengine
#DEFINES -= webkit
#DEFINES -= webengine
#DEFINES += webminiblink

contains(DEFINES, webkit) {
QT += webkit
greaterThan(QT_MAJOR_VERSION, 4) {
QT += webkitwidgets
}
} else {
contains(DEFINES, webengine) {
QT += webenginewidgets
} else {
win32 {DEFINES += webminiblink}
greaterThan(QT_MAJOR_VERSION, 4) {
lessThan(QT_MAJOR_VERSION, 6) {
win32 {QT += axcontainer}
}
} else {
win32 {CONFIG += qaxcontainer}
}}}

#miniblink需要主动引入user库否则在高版本msvc编译器通不过
contains(DEFINES, webminiblink) {
win32 {LIBS += -luser32}
}

#没有定义浏览器控件则全部移除标记
contains(DEFINES, nowebview) {
DEFINES -= webkit webengine webminiblink
QT -= webkit webkitwidgets webenginewidgets
}
