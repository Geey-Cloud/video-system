#include "frmvideopanel.h"
#include "ui_frmvideopanel.h"
#include "qthelper.h"
#include "commonkey.h"
#include "dbquery.h"
#include "devicehelper.h"
#include "deviceonvif.h"
#include "devicevideosmart.h"
#include "devicethreadui.h"
#include "videobox.h"
#include "videomanage.h"
#include "osdgraph.h"
#include "frmvideopaneltool.h"
#include "frmvideowidgetvolume.h"

frmVideoPanel::frmVideoPanel(QWidget *parent) : QWidget(parent), ui(new Ui::frmVideoPanel)
{
    ui->setupUi(this);
    this->initForm();
    this->initIcon();
    this->initVideo();
    this->initVideoManage();
    this->initOther();
    this->initMenu();
}

frmVideoPanel::~frmVideoPanel()
{
    delete ui;
}

void frmVideoPanel::mousePressEvent(QMouseEvent *e)
{
    //没有启用视频拖动则不处理
    if (!AppConfig::VideoDrag || AppConfig::VolumePosition > 0) {
        return;
    }

    //有通道开启了电子放大则不处理
    foreach (VideoWidget *w, videoWidgets) {
        if (w->getIsCrop()) {
            return;
        }
    }

    //过滤下如果按下的是底部工具栏则不处理
    if (!videoPanelTool->geometry().contains(e->pos())) {
        videoPressed = true;
        videoMove = false;
    }
}

void frmVideoPanel::mouseReleaseEvent(QMouseEvent *)
{
    if (!videoMove) {
        videoPressed = false;
        videoMove = false;
        videoSwap = 0;
        this->setCursor(Qt::ArrowCursor);
        return;
    }

    //如果从所有视频画面中没有找到则说明移出到窗体外
    if (!videoExist) {
        videoSelect->stop();
        int index = videoWidgets.indexOf(videoSelect);
        AppData::MediaUrls[index] = "";
        DeviceHelper::saveUrls();
        DeviceHelper::addMsg(QString("%1 删除视频").arg(videoSelect->getBgText()), 0, true);
    } else {
        //立即交换两个窗体信息
        if (videoSwap && videoSwap != videoSelect) {
            int channel1 = videoWidgets.indexOf(videoSelect);
            int channel2 = videoWidgets.indexOf(videoSwap);
            QMetaObject::invokeMethod(videoPanelTool, "changeVideo", Q_ARG(int, channel1), Q_ARG(int, channel2));
        }
    }

    videoPressed = false;
    videoMove = false;
    videoSwap = 0;

    //改变鼠标形状
    this->setCursor(Qt::ArrowCursor);
}

void frmVideoPanel::mouseMoveEvent(QMouseEvent *e)
{
    if (!videoPressed) {
        return;
    }

    videoMove = true;
    this->setCursor(QCursor(pixCursor));
    //先将窗体的坐标转换为视频窗体的坐标
    QPoint globalPos = e->globalPos();

    //逐个从所有的窗体比较判断当前是否是属于某个播放窗体
    videoExist = false;
    foreach (QWidget *widget, videoWidgets) {
        //如果控件不可见则继续,必须是可见的控件,比如8通道有1-8和9-16两种,通道1和通道9位置一样
        if (!widget->isVisible()) {
            continue;
        }

        QPoint widgetPos = this->mapToGlobal(widget->pos());
        QRect rect(widgetPos.x(), widgetPos.y(), widget->width(), widget->height());
        if (rect.contains(globalPos)) {
            videoSwap = (VideoWidget *)widget;
            videoExist = true;
            break;
        }
    }
}

bool frmVideoPanel::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::FocusIn) {
        //选中通道后通知节点通道选中
        VideoWidget *videoWidget = (VideoWidget *) watched;
        AppEvent::Instance()->slot_selectVideo(videoWidgets.indexOf(videoWidget));
    } else if (event->type() == QEvent::MouseButtonDblClick) {
        if (watched->inherits("QWidget")) {
            VideoWidget *videoWidget = (VideoWidget *) watched;
            QWidget *widget = videoWidget;
            if (videoWidget->parent()->inherits("frmVideoWidgetVolume")) {
                widget = (QWidget *)videoWidget->parent();
            }

            if (!AppConfig::VideoMax) {
                AppConfig::VideoMax = true;
                videoBox->hide_all();
                ui->gridLayout->addWidget(widget, 0, 0);
                widget->setVisible(true);
            } else {
                AppConfig::VideoMax = false;
                videoBox->show_all();
            }

            AppConfig::writeConfig();
            videoWidget->setFocus();
        }
    } else if (event->type() == QEvent::MouseButtonPress) {
        videoSelect = (VideoWidget *) watched;
        AppData::CurrentUrl = videoSelect->getVideoPara().mediaUrl;
        //在视频分割窗体弹出右键菜单
        if (qApp->mouseButtons() == Qt::RightButton) {
            videoMenu->exec(QCursor::pos());
        }
    }

    return QWidget::eventFilter(watched, event);
}

void frmVideoPanel::initForm()
{
    timerRestart = new QTimer(this);
    connect(timerRestart, SIGNAL(timeout()), this, SLOT(restartVideo2()));

    //初始化底部快捷工具栏
    videoPanelTool = new frmVideoPanelTool;
    videoPanelTool->setFixedHeight(AppData::ToolHeight);
    ui->widget->layout()->addWidget(videoPanelTool);

    //设置监控布局边距间距
    //非悬浮则需要留出一定的间隙/否则和其他通道挨得紧紧的/不能直观区分是哪个通道
    if (AppConfig::VolumePosition > 0 && AppConfig::VolumePosition < 6) {
        ui->gridLayout->setContentsMargins(6, 6, 6, 6);
        ui->gridLayout->setSpacing(3);
    }

    // ui->widget->setStyleSheet("background-color: #000000;");

    //启动定时器处理鼠标多久未操作自动全屏
    QTimer *timerAutoFull = new QTimer(this);
    connect(timerAutoFull, SIGNAL(timeout()), this, SLOT(autoFull()));
    timerAutoFull->start(1000);

    //关联样式改变信号自动重新设置图标
    connect(AppEvent::Instance(), SIGNAL(changeStyle()), this, SLOT(initIcon()));
    //绑定重新加载所有通道
    connect(AppEvent::Instance(), SIGNAL(restartVideo()), this, SLOT(restartVideo()));
    //应用程序关闭前先关闭所有视频
    connect(AppEvent::Instance(), SIGNAL(exitAll()), this, SLOT(closeAll()));
}

void frmVideoPanel::initIcon()
{
    //从图形字体获取鼠标按下拖动形状
    pixCursor = IconHelper::getPixmap(GlobalConfig::TextColor, 0xea3e, 20, 25, 25);
}

void frmVideoPanel::initVideo()
{
    videoExist = false;
    videoPressed = false;
    videoMove = false;
    videoSelect = 0;
    videoSwap = 0;

    //视频控件参数配置更改立即执行
    connect(AppEvent::Instance(), SIGNAL(changeVideoConfig()), this, SLOT(changeVideoConfig()));
    //视频管理参数配置更改立即执行
    connect(AppEvent::Instance(), SIGNAL(changeVideoManage()), this, SLOT(initVideoManage()));

    for (int i = 0; i < AppConfig::VideoCount; ++i) {
        VideoWidget *videoWidget = new VideoWidget;
        //OsdGraph::addDateTimeOsd(videoWidget);

        int index = i + 1;
        QString flag = QString("%1").arg(index, 2, 10, QChar('0'));
        QString objName = QString("ch%1").arg(flag);
        videoWidget->setObjectName(objName);

        //设置背景文字
        QString name = AppConfig::DefaultChName;
        QString bgText = QString("%1 %2").arg(name).arg(index);
        if (AppConfig::ChannelBgText == 1) {
            bgText = QString("%1 %2").arg(name).arg(index, 2, 10, QChar('0'));
        } else if (AppConfig::ChannelBgText == 2) {
            if (i < DbData::IpcInfo_Count) {
                bgText = DbData::IpcInfo_IpcName.at(i);
            }
        }

        videoWidget->setBgText(bgText);
        videoWidget->installEventFilter(this);
        videoWidget->setStyleSheet("border: 1px solid black;");

        //设置音量和静音状态
        videoWidget->setVolume(AppConfig::SoundValue);
        videoWidget->setMuted(AppConfig::SoundMuted);

        //绑定按下处像素坐标信号
        connect(videoWidget, SIGNAL(sig_receivePoint(int, QPoint)), AppEvent::Instance(), SIGNAL(receivePoint(int, QPoint)));
        //绑定播放开始信号
        connect(videoWidget, SIGNAL(sig_receivePlayStart(int)), DeviceOnvif::Instance(), SLOT(receivePlayStart(int)));
        //绑定播放结束信号
        connect(videoWidget, SIGNAL(sig_receivePlayFinsh()), DeviceOnvif::Instance(), SLOT(receivePlayFinsh()));
        //绑定拖放信号
        connect(videoWidget, SIGNAL(sig_fileDrag(QString)), this, SLOT(fileDrag(QString)));
        //绑定悬浮条按钮单击信号
        connect(videoWidget, SIGNAL(sig_btnClicked(QString)), this, SLOT(btnClicked(QString)));

        AppData::MediaUrls << "";
        videoWidgets << videoWidget;
    }

    //初始化上一次打开的地址列表
    int count = DeviceHelper::initUrls() - 1;
    //校验秘钥控制的设备数量
    if (!CommonKey::checkCount(count)) {
        //主动删除记录文件重来(防止一旦超过数量每次打开都弹窗提示而且无法运行)
        DeviceHelper::clearUrls();
        exit(0);
    }

    //可以在这里主动写死通道媒体地址(防止用户删了配置文件以及摄像头信息)
    //AppData::MediaUrls[0] = "f:/mp4/push/1.mp4";

    //设置当前控件为第一个控件
    videoSelect = videoWidgets.first();
    //初始化视频控件参数
    this->changeVideoConfig();
}

void frmVideoPanel::changeVideoConfig()
{
    for (int i = 0; i < AppConfig::VideoCount; ++i) {
        DeviceHelper::initVideoWidget(videoWidgets.at(i), AppConfig::SoundValue, AppConfig::SoundMuted, true, AppConfig::ReadTimeout);
    }
}

void frmVideoPanel::initVideoManage()
{
    VideoManage::Instance()->setMediaUrls(AppData::MediaUrls);
    VideoManage::Instance()->setVideoWidgets(videoWidgets);
    VideoManage::Instance()->setSaveVideo(AppConfig::SaveVideo);
    VideoManage::Instance()->setRecordTimes(DbQuery::getRecordTimes());
    VideoManage::Instance()->setPath(AppConfig::VideoNormalPath, AppConfig::ImageNormalPath);
}

void frmVideoPanel::initOther()
{
    //人工智能处理模板/有多少个需要就new多少个类出来
    for (int i = 0; i < AppConfig::VideoCount; ++i) {
        new DeviceVideoSmart(videoWidgets.at(i), this);
    }

#if 0
    //演示线程更新UI
    DeviceThreadUI *threadUI = new DeviceThreadUI(this);
    QTimer::singleShot(5000, threadUI, SLOT(start()));
#endif
}

void frmVideoPanel::restartVideo()
{
    //初始化视频配置参数到视频控件
    this->initVideoManage();
    //关闭所有视频通道
    this->closeAll();
    //定时器延时执行打开,用定时器的好处是防止重复执行可以立即取消前一次
    timerRestart->stop();
    timerRestart->start(1000);
}

void frmVideoPanel::restartVideo2()
{
    timerRestart->stop();
    this->changeVideoConfig();
    VideoManage::Instance()->start();
}

void frmVideoPanel::initMenu()
{
    videoMenu = new QMenu(this);

    actionFull = new QAction("切换全屏模式", videoMenu);
    connect(actionFull, SIGNAL(triggered(bool)), this, SLOT(full()));
    actionPoll = new QAction("启动轮询视频", videoMenu);
    connect(actionPoll, SIGNAL(triggered(bool)), this, SIGNAL(pollStart()));

    videoMenu->addAction(actionFull);
    videoMenu->addAction(actionPoll);
    //videoMenu->addAction("网页预览视频", this, SLOT(preview()));
    videoMenu->addSeparator();

    videoMenu->addAction("删除当前视频", this, SLOT(deleteOne()));
    videoMenu->addAction("删除所有视频", this, SLOT(deleteAll()));
    videoMenu->addSeparator();

    videoMenu->addAction("截图当前视频", this, SLOT(snapOne()));
    videoMenu->addAction("截图所有视频", this, SLOT(snapAll()));
    videoMenu->addSeparator();

    //定义哪些菜单不需要显示
    QList<bool> visibles;
    visibles << AppConfig::EnableChannelx << AppConfig::EnableChannely;
    visibles << AppConfig::EnableChannel4 << AppConfig::EnableChannel6 << AppConfig::EnableChannel8;
    visibles << AppConfig::EnableChannel9 << AppConfig::EnableChannel13 << AppConfig::EnableChannel16;
    visibles << AppConfig::EnableChannel25 << AppConfig::EnableChannel36 << AppConfig::EnableChannel64;

    //实例化通道布局类
    videoBox = new VideoBox(this);
    connect(videoBox, SIGNAL(changeLayout(int, QString, bool)), this, SLOT(changeLayout(int, QString, bool)));
    videoBox->setLayoutType(AppConfig::LayoutType);
    videoBox->setLayout(ui->gridLayout);
    videoBox->setVisibles(visibles);
    videoBox->initMenu(videoMenu);
    this->setWidgets();

    //控制底部面板是否可见
    videoPanelTool->setVisible(AppConfig::EnableVideoPanelTool);
}

void frmVideoPanel::setWidgets()
{
    //重新转换类型
    QWidgetList widgets;
    foreach (VideoWidget *videoWidget, this->videoWidgets) {
        //直接加入视频控件或者加入改装后的带音频柱状条的视频控件
        if (AppConfig::VolumePosition > 0) {
            widgets << new frmVideoWidgetVolume(AppConfig::VolumePosition, videoWidget);
        } else {
            widgets << videoWidget;
        }
    }

    videoBox->setWidgets(widgets);
    videoBox->show_all();
}

void frmVideoPanel::full()
{
    doFull(actionFull->text() == "切换全屏模式");
}

void frmVideoPanel::autoFull()
{
    //已经全屏或者设置的时间为0表示不启用
    if (AppConfig::FormFull || AppConfig::TimeAutoFull == 0 || !isVisible()) {
        return;
    }

    //计算鼠标未操作到现在的时间
    QDateTime now = QDateTime::currentDateTime();
    if (AppData::LastLiveTime.secsTo(now) > AppConfig::TimeAutoFull) {
        doFull(true);
    }
}

void frmVideoPanel::doFull(bool full)
{
    if (full) {
        actionFull->setText("切换正常模式");
        videoPanelTool->setVisible(false);
    } else {
        actionFull->setText("切换全屏模式");
        videoPanelTool->setVisible(AppConfig::EnableVideoPanelTool);
    }

    AppConfig::FormFull = full;
    AppConfig::writeConfig();
    AppEvent::Instance()->slot_fullScreen(full);
}

void frmVideoPanel::play(const QStringList &urls)
{
    VideoManage::Instance()->setMediaUrls(urls);
    VideoManage::Instance()->start();
}

void frmVideoPanel::playAll()
{
    play(AppData::MediaUrls);
}

void frmVideoPanel::closeAll()
{
    //轮询状态不提示
    if (AppConfig::Polling) {
        VideoManage::Instance()->stop();
        return;
    }

    //弹出提示信息窗体等待加载完成
    QtHelper::showSplashInfo("正在关闭所有通道, 请勿关闭程序...", 50, 15);
    qApp->processEvents();
    qApp->setOverrideCursor(Qt::WaitCursor);
    VideoManage::Instance()->stop();
    qApp->restoreOverrideCursor();
    QtHelper::hideSplashInfo();
}

void frmVideoPanel::deleteOne()
{
    //清空当前url以后云台控制失效(意味着必须打开的时候才能云台控制)
    AppData::CurrentUrl = "";
    if (videoSelect) {
        int index = videoWidgets.indexOf(videoSelect);
        QString url = AppData::MediaUrls.at(index);
        if (url.isEmpty()) {
            return;
        }

        //重新替换当前url并保存
        AppData::MediaUrls[index] = "";
        DeviceHelper::saveUrls();
        DeviceHelper::addMsg(QString("通道 %1 删除视频").arg(index + 1), 0, true);
        videoWidgets.at(index)->stop();
    }
}

void frmVideoPanel::deleteAll()
{
    closeAll();

    //替换所有url为空并保存
    for (int i = 0; i < AppConfig::VideoCount; ++i) {
        AppData::MediaUrls[i] = "";
    }

    DeviceHelper::saveUrls();
    DeviceHelper::addMsg(QString("所有通道 删除视频"), 0, true);
    //清空当前url以后云台控制失效,意味着必须打开的时候才能云台控制
    AppData::CurrentUrl = "";
}

void frmVideoPanel::snapOne()
{
    if (videoSelect) {
        QString objName = videoSelect->objectName();
        QString fileName = QString("%1/%2_%3.jpg").arg(VideoWidget::snapPath).arg(objName).arg(STRDATETIMEMS);
        videoSelect->snap(fileName);
        QtHelper::showTipBox("提示", QString("截图已保存到文件\n%1").arg(fileName), AppConfig::FullScreen, false, 5);
    }
}

void frmVideoPanel::snapAll()
{
    for (int index = 0; index < AppConfig::VideoCount; index++) {
        QString objName = videoWidgets.at(index)->objectName();
        QString fileName = QString("%1/%2_%3.jpg").arg(VideoWidget::snapPath).arg(objName).arg(STRDATETIMEMS);
        videoWidgets.at(index)->snap(fileName);
    }

    QtHelper::showTipBox("提示", QString("截图已保存到文件夹\n%1").arg(VideoWidget::snapPath), AppConfig::FullScreen, false, 5);
}

void frmVideoPanel::preview()
{
    QStringList mediaUrls;
    foreach (QString url, AppData::MediaUrls) {
        if (!url.isEmpty()) {
            mediaUrls << url;
        }
    }

    DeviceHelper::preview(0, "127.0.0.1", 9999, mediaUrls);
}

void frmVideoPanel::btnClicked(const QString &btnName)
{
    //识别是哪个通道上的哪个按钮
    VideoWidget *videoWidget = (VideoWidget *)sender();
    DeviceHelper::addMsg(QString("按下按钮 %1").arg(btnName));
    if (btnName.endsWith("btnClose")) {
        //关闭以后立即更新url.txt
        int index = videoWidgets.indexOf(videoWidget);
        if (index >= 0) {
            videoWidgets.at(index)->setFocus();
            AppData::MediaUrls[index] = "";
            DeviceHelper::saveUrls();
            DeviceHelper::addMsg(QString("通道 %1 关闭视频").arg(index + 1), 0, true);
        }
    }
}

void frmVideoPanel::fileDrag(const QString &url)
{
    //接收到的文件立即更新url.txt
    VideoWidget *videoWidget = (VideoWidget *)sender();
    int index = videoWidgets.indexOf(videoWidget);
    if (index >= 0) {
        videoWidget->open(url);
        videoWidget->setFocus();
        AppData::MediaUrls[index] = url;
        DeviceHelper::saveUrls();
        DeviceHelper::addMsg(QString("通道 %1 拖曳视频").arg(index + 1), 0, true);
        QTimer::singleShot(10, AppEvent::Instance(), SIGNAL(fileDragOpen()));
    }
}

void frmVideoPanel::changeLayout(int type, const QString &layoutType, bool max)
{
    AppConfig::LayoutType = layoutType;
    AppConfig::VideoMax = max;
    AppConfig::writeConfig();
    //底部工具栏通道切换按钮选中
    QMetaObject::invokeMethod(videoPanelTool, "changeLayout", Q_ARG(int, type));
}
