#include "frmvideowidgetvolume.h"
#include "ui_frmvideowidgetvolume.h"
#include "qthelper.h"
#include "videowidgetx.h"
#include "barvolume.h"

frmVideoWidgetVolume::frmVideoWidgetVolume(int position, VideoWidget *videoWidget, QWidget *parent) : QWidget(parent), ui(new Ui::frmVideoWidgetVolume)
{
    ui->setupUi(this);

    this->position = position;
    this->videoWidget = videoWidget;

    isVertical = true;
    if (position == 2 || position == 4) {
        isVertical = false;
    }

    //实例化音柱控件并设置方向
    leftVolume = new BarVolume;
    rightVolume = new BarVolume;
    leftVolume->setVertical(isVertical);
    rightVolume->setVertical(isVertical);

    //实例化布局并设置边距间距
    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(3);

    //将音柱控件和视频控件插入到对应位置/可以自行拓展各种布局
    if (position == 1) {
        layout->addWidget(leftVolume, 0, 0);
        layout->addWidget(rightVolume, 0, 1);
        layout->addWidget(videoWidget, 0, 2);
    } else if (position == 2) {
        layout->addWidget(leftVolume, 0, 0);
        layout->addWidget(rightVolume, 1, 0);
        layout->addWidget(videoWidget, 2, 0);
    } else if (position == 3) {
        layout->addWidget(videoWidget, 0, 0);
        layout->addWidget(leftVolume, 0, 1);
        layout->addWidget(rightVolume, 0, 2);
    } else if (position == 4) {
        layout->addWidget(videoWidget, 0, 0);
        layout->addWidget(leftVolume, 1, 0);
        layout->addWidget(rightVolume, 2, 0);
    } else if (position == 5) {
        layout->addWidget(leftVolume, 0, 0);
        layout->addWidget(videoWidget, 0, 1);
        layout->addWidget(rightVolume, 0, 2);
    } else if (position == 6) {
        //实例化弹簧控件
        spaceLeft = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred);
        spaceCenter = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred);
        spaceRight = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred);

        //实例化布局并插入控件
        hlayout = new QHBoxLayout;
        hlayout->addWidget(leftVolume);
        hlayout->addItem(spaceCenter);
        hlayout->addWidget(rightVolume);
        videoWidget->setLayout(hlayout);
        layout->addWidget(videoWidget);
    }

    //关联音频振幅信号
    connect(videoWidget, SIGNAL(sig_receiveLevel(qreal, qreal)), this, SLOT(receiveLevel(qreal, qreal)));
    //打开和关闭后需要清空下(有可能残留上一次的数据)
    connect(videoWidget, SIGNAL(sig_receivePlayStart(int)), this, SLOT(reset()));
    connect(videoWidget, SIGNAL(sig_receivePlayFinsh()), this, SLOT(reset()));
    //音柱窗体参数变化后需要重置
    connect(AppEvent::Instance(), SIGNAL(changeVolumeWidget()), this, SLOT(reset()));
}

frmVideoWidgetVolume::~frmVideoWidgetVolume()
{
    delete ui;
}

void frmVideoWidgetVolume::resizeEvent(QResizeEvent *)
{
    int width = this->width();
    int height = this->height();

    //根据宽度动态设置音量条宽度或者高度/也可以设置成固定值
    int size = (isVertical ? width : height);
    if (AppConfig::VolumeSize == 0) {
        size = size * 0.05;
        size = (size > 20 ? 20 : size);
    } else {
        size = AppConfig::VolumeSize;
    }

    if (isVertical) {
        leftVolume->setMaximumWidth(size);
        rightVolume->setMaximumWidth(size);
    } else {
        leftVolume->setMaximumHeight(size);
        rightVolume->setMaximumHeight(size);
    }

    //根据高度动态设置音量格子数量(一般高度越小格子数越少)
    int step = height / (position == 6 ? 10 : 6);
    leftVolume->setStep(step);
    rightVolume->setStep(step);

    //悬浮音柱需要动态调整
    if (position != 6) {
        return;
    }

    //线程未运行不用继续
    if (!videoWidget->getIsRunning()) {
        leftVolume->hide();
        rightVolume->hide();
        return;
    }

    //未打开成功不用继续
    VideoThread *videoThread = videoWidget->getVideoThread();
    if (!videoThread->getIsOk()) {
        leftVolume->hide();
        rightVolume->hide();
        return;
    }

    //没有音频不用继续
    if (videoThread->getAudioCodecName().isEmpty()) {
        return;
    }

    //根据音柱位置摆放控件
    leftVolume->show();
    rightVolume->show();

    //根据悬浮条的位置留出空余的地方
    int offset = 35;
    if (AppConfig::BannerPosition == 1) {
        hlayout->setContentsMargins(9, 9, 9, offset);
    } else if (AppConfig::BannerPosition == 2) {
        hlayout->setContentsMargins(offset, 9, 9, 9);
    } else if (AppConfig::BannerPosition == 3) {
        hlayout->setContentsMargins(9, 9, offset, 9);
    } else {
        hlayout->setContentsMargins(9, offset, 9, 9);
    }

    //先把布局中的挨个移除
    hlayout->removeItem(spaceLeft);
    hlayout->removeItem(spaceCenter);
    hlayout->removeItem(spaceRight);
    hlayout->removeWidget(leftVolume);
    hlayout->removeWidget(rightVolume);

    //纯音频需要占用比较大的面积
    if (videoWidget->getOnlyAudio()) {
        size = width / 4;
        leftVolume->setMaximumWidth(size);
        rightVolume->setMaximumWidth(size);
        leftVolume->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
        rightVolume->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
        hlayout->addItem(spaceLeft);
        hlayout->addWidget(leftVolume);
        hlayout->addWidget(rightVolume);
        hlayout->addItem(spaceRight);
    } else {
        leftVolume->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
        rightVolume->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
        hlayout->addWidget(leftVolume);
        hlayout->addItem(spaceCenter);
        hlayout->addWidget(rightVolume);
    }
}

void frmVideoWidgetVolume::reset()
{
    int alpha = AppConfig::VolumeBgAlpha;
    leftVolume->setBgColorAlpha(alpha);
    rightVolume->setBgColorAlpha(alpha);

    leftVolume->clear();
    rightVolume->clear();
    this->resizeEvent(NULL);
}

void frmVideoWidgetVolume::receiveLevel(qreal leftLevel, qreal rightLevel)
{
    leftVolume->setValue(leftLevel);
    rightVolume->setValue(rightLevel);
}
