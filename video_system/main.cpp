﻿#include "frmdemo.h"
#include "frmmain.h"
#include "frmlogin.h"

#include "qthelper.h"
#include "dbquery.h"
#include "appinit.h"

int main(int argc, char *argv[])
{
    int openGLType = QtHelper::getIniValue(argv, "OpenGLType", "config/", "video_system").toInt();
    QtHelper::initOpenGL(openGLType);

    bool use96Dpi = (QtHelper::getIniValue(argv, "Use96Dpi", "config/", "video_system") == "true");
    QtHelper::initMain(false, use96Dpi);

    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/main.ico"));

    //强制指定启动窗体方便测试
    AppConfig::IndexStart = 0;
    AppInit::Instance()->start();

    //根据不同的启动窗体索引来启动 0-主程序 1-演示示例
    QWidget *w;
    if (AppConfig::IndexStart == 1) {
        w = new frmDemo;
        w->setWindowTitle("监控系统组件示例 (QQ: 517216493  WX: feiyangqingyun)");
    } else {
        //开启了自动登录则直接启动主窗体/否则启动登录界面
        AppConfig::AutoLogin ? (w = new frmMain) : (w = new frmLogin);
    }

    w->show();
    w->activateWindow();
    return a.exec();
}
