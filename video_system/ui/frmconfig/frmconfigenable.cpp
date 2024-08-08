#include "frmconfigenable.h"
#include "ui_frmconfigenable.h"
#include "qthelper.h"
#include "appevent.h"

frmConfigEnable::frmConfigEnable(QWidget *parent) : QWidget(parent), ui(new Ui::frmConfigEnable)
{
    ui->setupUi(this);
    this->initEnableConfig1();
    this->initEnableConfig2();
    this->initEnableConfig3();
}

frmConfigEnable::~frmConfigEnable()
{
    delete ui;
}

void frmConfigEnable::initEnableConfig1()
{
    ui->listWidget1->item(0)->setCheckState(AppConfig::EnableChannelx ? Qt::Checked : Qt::Unchecked);
    ui->listWidget1->item(1)->setCheckState(AppConfig::EnableChannely ? Qt::Checked : Qt::Unchecked);
    ui->listWidget1->item(2)->setCheckState(AppConfig::EnableChannel1 ? Qt::Checked : Qt::Unchecked);
    ui->listWidget1->item(3)->setCheckState(AppConfig::EnableChannel4 ? Qt::Checked : Qt::Unchecked);
    ui->listWidget1->item(4)->setCheckState(AppConfig::EnableChannel6 ? Qt::Checked : Qt::Unchecked);
    ui->listWidget1->item(5)->setCheckState(AppConfig::EnableChannel8 ? Qt::Checked : Qt::Unchecked);
    ui->listWidget1->item(6)->setCheckState(AppConfig::EnableChannel9 ? Qt::Checked : Qt::Unchecked);
    ui->listWidget1->item(7)->setCheckState(AppConfig::EnableChannel13 ? Qt::Checked : Qt::Unchecked);
    ui->listWidget1->item(8)->setCheckState(AppConfig::EnableChannel16 ? Qt::Checked : Qt::Unchecked);
    ui->listWidget1->item(9)->setCheckState(AppConfig::EnableChannel25 ? Qt::Checked : Qt::Unchecked);
    ui->listWidget1->item(10)->setCheckState(AppConfig::EnableChannel36 ? Qt::Checked : Qt::Unchecked);
    ui->listWidget1->item(11)->setCheckState(AppConfig::EnableChannel64 ? Qt::Checked : Qt::Unchecked);

    //关联按下行自动切换选中状态
    connect(ui->listWidget1, SIGNAL(itemPressed(QListWidgetItem *)), AppEvent::Instance(), SLOT(slot_itemPressed(QListWidgetItem *)));
    connect(ui->listWidget1, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(saveEnableConfig1()));
}

void frmConfigEnable::saveEnableConfig1()
{
    AppConfig::EnableChannelx = (ui->listWidget1->item(0)->checkState() == Qt::Checked);
    AppConfig::EnableChannely = (ui->listWidget1->item(1)->checkState() == Qt::Checked);
    AppConfig::EnableChannel1 = (ui->listWidget1->item(2)->checkState() == Qt::Checked);
    AppConfig::EnableChannel4 = (ui->listWidget1->item(3)->checkState() == Qt::Checked);
    AppConfig::EnableChannel6 = (ui->listWidget1->item(4)->checkState() == Qt::Checked);
    AppConfig::EnableChannel8 = (ui->listWidget1->item(5)->checkState() == Qt::Checked);
    AppConfig::EnableChannel9 = (ui->listWidget1->item(6)->checkState() == Qt::Checked);
    AppConfig::EnableChannel13 = (ui->listWidget1->item(7)->checkState() == Qt::Checked);
    AppConfig::EnableChannel16 = (ui->listWidget1->item(8)->checkState() == Qt::Checked);
    AppConfig::EnableChannel25 = (ui->listWidget1->item(9)->checkState() == Qt::Checked);
    AppConfig::EnableChannel36 = (ui->listWidget1->item(10)->checkState() == Qt::Checked);
    AppConfig::EnableChannel64 = (ui->listWidget1->item(11)->checkState() == Qt::Checked);
    AppConfig::writeConfig();
}

void frmConfigEnable::initEnableConfig2()
{
    ui->listWidget2->item(0)->setCheckState(AppConfig::EnableMainVideo ? Qt::Checked : Qt::Unchecked);
    ui->listWidget2->item(1)->setCheckState(AppConfig::EnableMainMap ? Qt::Checked : Qt::Unchecked);
    ui->listWidget2->item(2)->setCheckState(AppConfig::EnableMainData ? Qt::Checked : Qt::Unchecked);
    ui->listWidget2->item(3)->setCheckState(AppConfig::EnableMainOther ? Qt::Checked : Qt::Unchecked);

    ui->listWidget2->item(4)->setCheckState(AppConfig::EnableMapImage ? Qt::Checked : Qt::Unchecked);
    ui->listWidget2->item(5)->setCheckState(AppConfig::EnableVideoPlayWeb ? Qt::Checked : Qt::Unchecked);
    ui->listWidget2->item(6)->setCheckState(AppConfig::EnableVideoPlayRtsp ? Qt::Checked : Qt::Unchecked);
    ui->listWidget2->item(7)->setCheckState(AppConfig::EnableVideoPlayImage ? Qt::Checked : Qt::Unchecked);
    ui->listWidget2->item(8)->setCheckState(AppConfig::EnableVideoUpload ? Qt::Checked : Qt::Unchecked);
    ui->listWidget2->item(9)->setCheckState(AppConfig::EnableDataDevice ? Qt::Checked : Qt::Unchecked);
    ui->listWidget2->item(10)->setCheckState(AppConfig::EnableConfigPoll ? Qt::Checked : Qt::Unchecked);
    ui->listWidget2->item(11)->setCheckState(AppConfig::EnableConfigUser ? Qt::Checked : Qt::Unchecked);
    ui->listWidget2->item(12)->setCheckState(AppConfig::EnableConfigRecord ? Qt::Checked : Qt::Unchecked);
    ui->listWidget2->item(13)->setCheckState(AppConfig::EnableConfigOther ? Qt::Checked : Qt::Unchecked);

    ui->listWidget2->item(14)->setCheckState(AppConfig::EnableWeather ? Qt::Checked : Qt::Unchecked);
    ui->listWidget2->item(15)->setCheckState(AppConfig::EnableVideoPanelTool ? Qt::Checked : Qt::Unchecked);
    ui->listWidget2->item(16)->setCheckState(AppConfig::EnableTitleMenu ? Qt::Checked : Qt::Unchecked);
    ui->listWidget2->item(17)->setCheckState(AppConfig::EnableDockMenu ? Qt::Checked : Qt::Unchecked);
    ui->listWidget2->item(18)->setCheckState(AppConfig::EnableDockMove ? Qt::Checked : Qt::Unchecked);

    //关联按下行自动切换选中状态
    connect(ui->listWidget2, SIGNAL(itemPressed(QListWidgetItem *)), AppEvent::Instance(), SLOT(slot_itemPressed(QListWidgetItem *)));
    connect(ui->listWidget2, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(saveEnableConfig2()));
}

void frmConfigEnable::saveEnableConfig2()
{
    AppConfig::EnableMainVideo = (ui->listWidget2->item(0)->checkState() == Qt::Checked);
    AppConfig::EnableMainMap = (ui->listWidget2->item(1)->checkState() == Qt::Checked);
    AppConfig::EnableMainData = (ui->listWidget2->item(2)->checkState() == Qt::Checked);
    AppConfig::EnableMainOther = (ui->listWidget2->item(3)->checkState() == Qt::Checked);

    AppConfig::EnableMapImage = (ui->listWidget2->item(4)->checkState() == Qt::Checked);
    AppConfig::EnableVideoPlayWeb = (ui->listWidget2->item(5)->checkState() == Qt::Checked);
    AppConfig::EnableVideoPlayRtsp = (ui->listWidget2->item(6)->checkState() == Qt::Checked);
    AppConfig::EnableVideoPlayImage = (ui->listWidget2->item(7)->checkState() == Qt::Checked);
    AppConfig::EnableVideoUpload = (ui->listWidget2->item(8)->checkState() == Qt::Checked);
    AppConfig::EnableDataDevice = (ui->listWidget2->item(9)->checkState() == Qt::Checked);
    AppConfig::EnableConfigPoll = (ui->listWidget2->item(10)->checkState() == Qt::Checked);
    AppConfig::EnableConfigUser = (ui->listWidget2->item(11)->checkState() == Qt::Checked);
    AppConfig::EnableConfigRecord = (ui->listWidget2->item(12)->checkState() == Qt::Checked);
    AppConfig::EnableConfigOther = (ui->listWidget2->item(13)->checkState() == Qt::Checked);

    bool enableWeather = (ui->listWidget2->item(14)->checkState() == Qt::Checked);
    if (AppConfig::EnableWeather != enableWeather) {
        AppConfig::EnableWeather = enableWeather;
        AppEvent::Instance()->slot_changeWeatherVisible();
    }

    AppConfig::EnableVideoPanelTool = (ui->listWidget2->item(15)->checkState() == Qt::Checked);
    AppConfig::EnableTitleMenu = (ui->listWidget2->item(16)->checkState() == Qt::Checked);
    AppConfig::EnableDockMenu = (ui->listWidget2->item(17)->checkState() == Qt::Checked);
    AppConfig::EnableDockMove = (ui->listWidget2->item(18)->checkState() == Qt::Checked);
    AppConfig::writeConfig();
}

void frmConfigEnable::initEnableConfig3()
{
    ui->listWidget3->item(0)->setCheckState(AppConfig::EnableResolution ? Qt::Checked : Qt::Unchecked);
    ui->listWidget3->item(1)->setCheckState(AppConfig::EnableFrameRate ? Qt::Checked : Qt::Unchecked);
    ui->listWidget3->item(2)->setCheckState(AppConfig::EnableMediaUrl ? Qt::Checked : Qt::Unchecked);
    ui->listWidget3->item(3)->setCheckState(AppConfig::EnableFormatName ? Qt::Checked : Qt::Unchecked);
    ui->listWidget3->item(4)->setCheckState(AppConfig::EnableVideoCodecName ? Qt::Checked : Qt::Unchecked);
    ui->listWidget3->item(5)->setCheckState(AppConfig::EnableAudioCodecName ? Qt::Checked : Qt::Unchecked);
    ui->listWidget3->item(6)->setCheckState(AppConfig::EnableRealBitRate ? Qt::Checked : Qt::Unchecked);
    ui->listWidget3->item(7)->setCheckState(AppConfig::EnableHardware ? Qt::Checked : Qt::Unchecked);
    ui->listWidget3->item(8)->setCheckState(AppConfig::EnableVideoMode ? Qt::Checked : Qt::Unchecked);

    //关联按下行自动切换选中状态
    connect(ui->listWidget3, SIGNAL(itemPressed(QListWidgetItem *)), AppEvent::Instance(), SLOT(slot_itemPressed(QListWidgetItem *)));
    connect(ui->listWidget3, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(saveEnableConfig3()));
}

void frmConfigEnable::saveEnableConfig3()
{
    AppConfig::EnableResolution = (ui->listWidget3->item(0)->checkState() == Qt::Checked);
    AppConfig::EnableFrameRate = (ui->listWidget3->item(1)->checkState() == Qt::Checked);
    AppConfig::EnableMediaUrl = (ui->listWidget3->item(2)->checkState() == Qt::Checked);
    AppConfig::EnableFormatName = (ui->listWidget3->item(3)->checkState() == Qt::Checked);
    AppConfig::EnableVideoCodecName = (ui->listWidget3->item(4)->checkState() == Qt::Checked);
    AppConfig::EnableAudioCodecName = (ui->listWidget3->item(5)->checkState() == Qt::Checked);
    AppConfig::EnableRealBitRate = (ui->listWidget3->item(6)->checkState() == Qt::Checked);
    AppConfig::EnableHardware = (ui->listWidget3->item(7)->checkState() == Qt::Checked);
    AppConfig::EnableVideoMode = (ui->listWidget3->item(8)->checkState() == Qt::Checked);
    AppConfig::writeConfig();

    //这些参数可以立即应用
    AppEvent::Instance()->slot_changeVideoConfig();
}
