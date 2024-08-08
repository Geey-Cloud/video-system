#include "frmconfigvideo.h"
#include "ui_frmconfigvideo.h"
#include "qthelper.h"
#include "videoutil.h"
#include "videohelper.h"
#include "onvifdevice.h"
#include "abstractsavethread.h"
#include "abstractvideothread.h"

frmConfigVideo::frmConfigVideo(QWidget *parent) : QWidget(parent), ui(new Ui::frmConfigVideo)
{
    ui->setupUi(this);
    this->initDebugInfo();
    this->initVideoConfig1();
    this->initVideoConfig2();
    this->initVideoConfig3();
    this->initVideoConfig4();
}

frmConfigVideo::~frmConfigVideo()
{
    delete ui;
}

void frmConfigVideo::initDebugInfo()
{
    int type = (AppConfig::DebugInfo ? 2 : 0);
    OnvifDevice::debugInfo = type;
    AbstractSaveThread::debugInfo = type;
    AbstractVideoThread::debugInfo = type;
}

void frmConfigVideo::initVideoConfig1()
{
    ui->btnSharedData->setChecked(AppConfig::SharedData);
    connect(ui->btnSharedData, SIGNAL(checkedChanged(bool)), this, SLOT(saveVideoConfig1()));

    ui->btnDebugInfo->setChecked(AppConfig::DebugInfo);
    connect(ui->btnDebugInfo, SIGNAL(checkedChanged(bool)), this, SLOT(saveVideoConfig1()));

    VideoUtil::loadScaleMode(ui->cboxScaleMode);
    ui->cboxScaleMode->setCurrentIndex(AppConfig::ScaleMode);
    connect(ui->cboxScaleMode, SIGNAL(currentIndexChanged(int)), this, SLOT(saveVideoConfig1()));

    VideoUtil::loadVideoMode(ui->cboxVideoMode);
    ui->cboxVideoMode->setCurrentIndex(AppConfig::VideoMode);
    connect(ui->cboxVideoMode, SIGNAL(currentIndexChanged(int)), this, SLOT(saveVideoConfig1()));

    QStringList listBorderWidth, listBorderWidthx;
    listBorderWidth << "0 像素" << "2 像素" << "4 像素" << "6 像素" << "8 像素";
    listBorderWidthx << "0" << "2" << "4" << "6" << "8";

    int count = listBorderWidth.count();
    for (int i = 0; i < count; ++i) {
        ui->cboxBorderWidth->addItem(listBorderWidth.at(i), listBorderWidthx.at(i));
    }

    ui->cboxBorderWidth->setCurrentIndex(ui->cboxBorderWidth->findData(AppConfig::BorderWidth));
    connect(ui->cboxBorderWidth, SIGNAL(currentIndexChanged(int)), this, SLOT(saveVideoConfig1()));

    ui->cboxBannerPosition->setCurrentIndex(AppConfig::BannerPosition);
    connect(ui->cboxBannerPosition, SIGNAL(currentIndexChanged(int)), this, SLOT(saveVideoConfig1()));

    ui->sboxBannerBgAlpha->setRange(0, 255);
    ui->sboxBannerBgAlpha->setValue(AppConfig::BannerBgAlpha);
    connect(ui->sboxBannerBgAlpha, SIGNAL(valueChanged(int)), this, SLOT(saveVideoConfig1()));

    ui->sboxVolumeBgAlpha->setRange(0, 255);
    ui->sboxVolumeBgAlpha->setValue(AppConfig::VolumeBgAlpha);
    connect(ui->sboxVolumeBgAlpha, SIGNAL(valueChanged(int)), this, SLOT(saveVideoConfig1()));
}

void frmConfigVideo::saveVideoConfig1()
{
    bool sharedData = ui->btnSharedData->getChecked();
    if (AppConfig::SharedData != sharedData) {
        AppConfig::SharedData = sharedData;
        AppEvent::Instance()->slot_changeVideoConfig();
    }

    bool debugInfo = ui->btnDebugInfo->getChecked();
    if (AppConfig::DebugInfo != debugInfo) {
        AppConfig::DebugInfo = debugInfo;
        this->initDebugInfo();
    }

    int scaleMode = ui->cboxScaleMode->currentIndex();
    if (AppConfig::ScaleMode != scaleMode) {
        AppConfig::ScaleMode = scaleMode;
        AppEvent::Instance()->slot_changeVideoConfig();
    }

    AppConfig::VideoMode = ui->cboxVideoMode->currentIndex();

    int borderWidth = ui->cboxBorderWidth->itemData(ui->cboxBorderWidth->currentIndex()).toInt();
    if (AppConfig::BorderWidth != borderWidth) {
        AppConfig::BorderWidth = borderWidth;
        AppEvent::Instance()->slot_changeVideoConfig();
    }

    int bannerPosition = ui->cboxBannerPosition->currentIndex();
    if (AppConfig::BannerPosition != bannerPosition) {
        AppConfig::BannerPosition = bannerPosition;
        AppEvent::Instance()->slot_changeVideoConfig();
        AppEvent::Instance()->slot_changeVolumeWidget();
    }

    int bannerBgAlpha = ui->sboxBannerBgAlpha->value();
    if (AppConfig::BannerBgAlpha != bannerBgAlpha) {
        AppConfig::BannerBgAlpha = bannerBgAlpha;
        AppEvent::Instance()->slot_changeVideoConfig();
    }

    int volumeBgAlpha = ui->sboxVolumeBgAlpha->value();
    if (AppConfig::VolumeBgAlpha != volumeBgAlpha) {
        AppConfig::VolumeBgAlpha = volumeBgAlpha;
        AppEvent::Instance()->slot_changeVolumeWidget();
    }

    AppConfig::writeConfig();
}

void frmConfigVideo::initVideoConfig2()
{
    VideoCore videoCore = VideoHelper::getVideoCore();

    ui->btnPlayAudio->setChecked(AppConfig::PlayAudio);
    connect(ui->btnPlayAudio, SIGNAL(checkedChanged(bool)), this, SLOT(saveVideoConfig2()));

    VideoUtil::loadHardware(ui->cboxHardware, videoCore, AppConfig::Hardware);
    connect(ui->cboxHardware, SIGNAL(currentIndexChanged(int)), this, SLOT(saveVideoConfig2()));

    VideoUtil::loadTransport(ui->cboxTransport);
    ui->cboxTransport->setCurrentIndex(ui->cboxTransport->findText(AppConfig::Transport));
    connect(ui->cboxTransport, SIGNAL(currentIndexChanged(int)), this, SLOT(saveVideoConfig2()));

    VideoUtil::loadDecodeType(ui->cboxDecodeType);
    ui->cboxDecodeType->setCurrentIndex(AppConfig::DecodeType);
    connect(ui->cboxDecodeType, SIGNAL(currentIndexChanged(int)), this, SLOT(saveVideoConfig2()));

    VideoUtil::loadEncodeVideo(ui->cboxEncodeVideo);
    ui->cboxEncodeVideo->setCurrentIndex(AppConfig::EncodeVideo);
    connect(ui->cboxEncodeVideo, SIGNAL(currentIndexChanged(int)), this, SLOT(saveVideoConfig2()));

    VideoUtil::loadCaching(ui->cboxCaching, videoCore, AppConfig::Caching);
    connect(ui->cboxCaching, SIGNAL(currentIndexChanged(int)), this, SLOT(saveVideoConfig2()));

    VideoUtil::loadReadTimeout(ui->cboxReadTimeout);
    ui->cboxReadTimeout->setCurrentIndex(ui->cboxReadTimeout->findData(AppConfig::ReadTimeout));
    connect(ui->cboxReadTimeout, SIGNAL(currentIndexChanged(int)), this, SLOT(saveVideoConfig2()));

    VideoUtil::loadConnectTimeout(ui->cboxConnectTimeout);
    ui->cboxConnectTimeout->setCurrentIndex(ui->cboxConnectTimeout->findData(AppConfig::ConnectTimeout));
    connect(ui->cboxConnectTimeout, SIGNAL(currentIndexChanged(int)), this, SLOT(saveVideoConfig2()));
}

void frmConfigVideo::saveVideoConfig2()
{
    bool playAudio = ui->btnPlayAudio->getChecked();
    if (AppConfig::PlayAudio != playAudio) {
        AppConfig::PlayAudio = playAudio;
        AppEvent::Instance()->slot_changeVideoConfig();
    }

    AppConfig::Hardware = ui->cboxHardware->currentText();
    AppConfig::Transport = ui->cboxTransport->currentText();
    AppConfig::DecodeType = ui->cboxDecodeType->currentIndex();
    AppConfig::EncodeVideo = ui->cboxEncodeVideo->currentIndex();
    AppConfig::Caching = ui->cboxCaching->itemData(ui->cboxCaching->currentIndex()).toInt();
    AppConfig::ReadTimeout = ui->cboxReadTimeout->itemData(ui->cboxReadTimeout->currentIndex()).toInt();
    AppConfig::ConnectTimeout = ui->cboxConnectTimeout->itemData(ui->cboxConnectTimeout->currentIndex()).toInt();
    AppConfig::writeConfig();
}

void frmConfigVideo::initVideoConfig3()
{
    ui->btnVideoDrag->setChecked(AppConfig::VideoDrag);
    connect(ui->btnVideoDrag, SIGNAL(checkedChanged(bool)), this, SLOT(saveVideoConfig3()));

    ui->btnSaveVideo->setChecked(AppConfig::SaveVideo);
    connect(ui->btnSaveVideo, SIGNAL(checkedChanged(bool)), this, SLOT(saveVideoConfig3()));

    ui->btnVideoSmart->setChecked(AppConfig::VideoSmart);
    connect(ui->btnVideoSmart, SIGNAL(checkedChanged(bool)), this, SLOT(saveVideoConfig3()));

    ui->btnOnvifEvent->setChecked(AppConfig::OnvifEvent);
    connect(ui->btnOnvifEvent, SIGNAL(checkedChanged(bool)), this, SLOT(saveVideoConfig3()));

    ui->btnOnvifNtp->setChecked(AppConfig::OnvifNtp);
    connect(ui->btnOnvifNtp, SIGNAL(checkedChanged(bool)), this, SLOT(saveVideoConfig3()));
}

void frmConfigVideo::saveVideoConfig3()
{
    //存储录像开启与关闭都要更新录像计划
    bool saveVideo = ui->btnSaveVideo->getChecked();
    if (AppConfig::SaveVideo != saveVideo) {
        AppConfig::SaveVideo = saveVideo;
        AppEvent::Instance()->slot_changeVideoManage();
    }

    AppConfig::VideoDrag = ui->btnVideoDrag->getChecked();
    AppConfig::VideoSmart = ui->btnVideoSmart->getChecked();
    AppConfig::OnvifEvent = ui->btnOnvifEvent->getChecked();
    AppConfig::OnvifNtp = ui->btnOnvifNtp->getChecked();
    AppConfig::writeConfig();
}

void frmConfigVideo::initVideoConfig4()
{
    ui->btnCheckOffline->setChecked(AppConfig::CheckOffline);
    connect(ui->btnCheckOffline, SIGNAL(checkedChanged(bool)), this, SLOT(saveVideoConfig4()));

    ui->cboxRtspType->setCurrentIndex(AppConfig::RtspType);
    connect(ui->cboxRtspType, SIGNAL(currentIndexChanged(int)), this, SLOT(saveVideoConfig4()));

    ui->cboxAlarmSaveTime->addItem("10 秒", 10);
    ui->cboxAlarmSaveTime->addItem("30 秒", 30);
    ui->cboxAlarmSaveTime->addItem("60 秒", 60);
    ui->cboxAlarmSaveTime->setCurrentIndex(ui->cboxAlarmSaveTime->findData(AppConfig::AlarmSaveTime));
    connect(ui->cboxAlarmSaveTime, SIGNAL(currentIndexChanged(int)), this, SLOT(saveVideoConfig4()));

    ui->cboxOpenIpcPolicy->addItem("最后空白通道");
    ui->cboxOpenIpcPolicy->addItem("最后按下通道");
    ui->cboxOpenIpcPolicy->setCurrentIndex(AppConfig::OpenIpcPolicy);
    connect(ui->cboxOpenIpcPolicy, SIGNAL(currentIndexChanged(int)), this, SLOT(saveVideoConfig4()));

    ui->cboxOpenNvrPolicy->addItem("最后空白通道");
    ui->cboxOpenNvrPolicy->addItem("从头清空通道");
    ui->cboxOpenNvrPolicy->setCurrentIndex(AppConfig::OpenNvrPolicy);
    connect(ui->cboxOpenNvrPolicy, SIGNAL(currentIndexChanged(int)), this, SLOT(saveVideoConfig4()));

    ui->cboxOpenMaxCount->addItem("最大通道数量");
    ui->cboxOpenMaxCount->addItem("当前通道数量");
    ui->cboxOpenMaxCount->addItem("自动调整数量");
    ui->cboxOpenMaxCount->setCurrentIndex(AppConfig::OpenMaxCount);
    connect(ui->cboxOpenMaxCount, SIGNAL(currentIndexChanged(int)), this, SLOT(saveVideoConfig4()));

    ui->cboxTableDataPolicy->addItem("单选按下编辑");
    ui->cboxTableDataPolicy->addItem("多选双击编辑");
    ui->cboxTableDataPolicy->setCurrentIndex(AppConfig::TableDataPolicy);
    connect(ui->cboxTableDataPolicy, SIGNAL(currentIndexChanged(int)), this, SLOT(saveVideoConfig4()));

    ui->cboxChannelBgText->addItem("0-通道1");
    ui->cboxChannelBgText->addItem("1-通道01");
    ui->cboxChannelBgText->addItem("2-设备名称");
    ui->cboxChannelBgText->setCurrentIndex(AppConfig::ChannelBgText);
    connect(ui->cboxChannelBgText, SIGNAL(currentIndexChanged(int)), this, SLOT(saveVideoConfig4()));

    ui->cboxVolumePosition->addItem("0-禁用");
    ui->cboxVolumePosition->addItem("1-左侧");
    ui->cboxVolumePosition->addItem("2-顶部");
    ui->cboxVolumePosition->addItem("3-右侧");
    ui->cboxVolumePosition->addItem("4-底部");
    ui->cboxVolumePosition->addItem("5-两侧");
    ui->cboxVolumePosition->addItem("6-悬浮");
    ui->cboxVolumePosition->setCurrentIndex(AppConfig::VolumePosition);
    connect(ui->cboxVolumePosition, SIGNAL(currentIndexChanged(int)), this, SLOT(saveVideoConfig4()));

    ui->cboxVolumeSize->addItem("00-自动", 0);
    ui->cboxVolumeSize->addItem("10-微小", 10);
    ui->cboxVolumeSize->addItem("20-常规", 20);
    ui->cboxVolumeSize->addItem("30-较大", 30);
    ui->cboxVolumeSize->addItem("50-极大", 50);

    //将用户指定的值添加到下拉框
    int size = AppConfig::VolumeSize;
    int index = ui->cboxVolumeSize->findData(size);
    if (index < 0) {
        index = ui->cboxVolumeSize->count();
        ui->cboxVolumeSize->addItem(QString("%1-指定").arg(size), size);
    }

    ui->cboxVolumeSize->setCurrentIndex(index);
    connect(ui->cboxVolumeSize, SIGNAL(currentIndexChanged(int)), this, SLOT(saveVideoConfig4()));
}

void frmConfigVideo::saveVideoConfig4()
{
    AppConfig::CheckOffline = ui->btnCheckOffline->getChecked();
    AppConfig::RtspType = ui->cboxRtspType->currentIndex();
    AppConfig::AlarmSaveTime = ui->cboxAlarmSaveTime->itemData(ui->cboxAlarmSaveTime->currentIndex()).toInt();
    AppConfig::OpenIpcPolicy = ui->cboxOpenIpcPolicy->currentIndex();
    AppConfig::OpenNvrPolicy = ui->cboxOpenNvrPolicy->currentIndex();
    AppConfig::OpenMaxCount = ui->cboxOpenMaxCount->currentIndex();
    AppConfig::TableDataPolicy = ui->cboxTableDataPolicy->currentIndex();
    AppConfig::ChannelBgText = ui->cboxChannelBgText->currentIndex();

    int volumePosition = ui->cboxVolumePosition->currentIndex();
    if (AppConfig::VolumePosition != volumePosition) {
        AppConfig::VolumePosition = volumePosition;
        AppConfig::writeConfig();
        FormHelper::reboot();
    }

    int volumeSize = ui->cboxVolumeSize->itemData(ui->cboxVolumeSize->currentIndex()).toInt();
    if (AppConfig::VolumeSize != volumeSize) {
        AppConfig::VolumeSize = volumeSize;
        AppEvent::Instance()->slot_changeVolumeWidget();
    }

    AppConfig::writeConfig();
}
