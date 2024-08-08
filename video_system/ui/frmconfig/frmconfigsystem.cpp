#include "frmconfigsystem.h"
#include "ui_frmconfigsystem.h"
#include "qthelper.h"
#include "appstyle.h"
#include "savelog.h"
#include "saveruntime.h"
#include "dbconnthread.h"
#include "devicehelper.h"

frmConfigSystem::frmConfigSystem(QWidget *parent) : QWidget(parent), ui(new Ui::frmConfigSystem)
{
    ui->setupUi(this);
    this->initForm();
}

frmConfigSystem::~frmConfigSystem()
{
    delete ui;
}

void frmConfigSystem::initForm()
{
    //设置左侧右侧面板的宽度
    ui->widgetLeft->setFixedWidth(480);
    ui->widgetRight->setFixedWidth(450);

    //设置数据库设置面板的参数
    //ui->widgetConfigDb->setBtnInRight(true);
    //ui->txtCopyright->setReadOnly(false);

    this->initAppConfig1();
    this->initAppConfig2();
    this->initAppConfig3();
    this->initColorConfig();
}

void frmConfigSystem::initAppConfig1()
{
    QStringList listWorkMode;
    listWorkMode << "视频监控" << "机器人监控" << "无人机监控" << "智慧校园" << "广播监控";
    ui->cboxWorkMode->addItems(listWorkMode);
    ui->cboxWorkMode->setCurrentIndex(AppConfig::WorkMode);
    connect(ui->cboxWorkMode, SIGNAL(currentIndexChanged(int)), this, SLOT(saveAppConfig1()));

    QStringList listNavStyle;
    listNavStyle << "左侧+左侧" << "左侧+上侧" << "上侧+左侧" << "上侧+上侧";
    ui->cboxNavStyle->addItems(listNavStyle);
    ui->cboxNavStyle->setCurrentIndex(AppConfig::NavStyle);
    connect(ui->cboxNavStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(saveAppConfig1()));

    QStringList styleNames, styleFiles;
    GlobalStyle::getStyle(styleNames, styleFiles);
    for (int i = 0; i < styleNames.count(); ++i) {
        ui->cboxStyleName->addItem(styleNames.at(i), styleFiles.at(i));
    }

    ui->cboxStyleName->setCurrentIndex(ui->cboxStyleName->findData(AppConfig::StyleName));
    connect(ui->cboxStyleName, SIGNAL(currentIndexChanged(int)), this, SLOT(saveAppConfig1()));

    //图形字体图标
    QStringList icons;
    icons << ":/logo.svg" << "icon_0xea2a" << "icon_0xe9d0" << "icon_0xe9cf" << "icon_0xe9d5" << "icon_0xea07" << "icon_0xf1d1";
    ui->cboxLogoImage->addItems(icons);

    //从logo目录下加载所有图片
    QDir dir(QtHelper::appPath() + "/logo");
    QStringList fileNames = dir.entryList(QStringList() << "*.png");
    foreach (QString fileName, fileNames) {
        ui->cboxLogoImage->addItem(fileName.split(".").first());
    }

    int index = ui->cboxLogoImage->findText(AppConfig::LogoImage);
    ui->cboxLogoImage->setCurrentIndex(index < 0 ? 0 : index);
    connect(ui->cboxLogoImage, SIGNAL(currentIndexChanged(int)), this, SLOT(saveAppConfig1()));

    ui->txtCopyright->setText(AppConfig::Copyright);
    connect(ui->txtCopyright, SIGNAL(textChanged(QString)), this, SLOT(saveAppConfig1()));

    ui->txtTitleCn->setText(AppConfig::TitleCn);
    connect(ui->txtTitleCn, SIGNAL(textChanged(QString)), this, SLOT(saveAppConfig1()));

    ui->txtTitleEn->setText(AppConfig::TitleEn);
    connect(ui->txtTitleEn, SIGNAL(textChanged(QString)), this, SLOT(saveAppConfig1()));
}

void frmConfigSystem::saveAppConfig1()
{
    int workMode = ui->cboxWorkMode->currentIndex();
    if (AppConfig::WorkMode != workMode) {
        AppConfig::WorkMode = workMode;
        FormHelper::reboot(false);
    }

    int navStyle = ui->cboxNavStyle->currentIndex();
    if (AppConfig::NavStyle != navStyle) {
        AppConfig::NavStyle = navStyle;
        FormHelper::reboot(false);
    }

    int styleIndex = ui->cboxStyleName->currentIndex();
    QString styleName = ui->cboxStyleName->itemData(styleIndex).toString();
    if (AppConfig::StyleName != styleName) {
        AppConfig::StyleName = styleName;
        AppStyle::initStyle();
        AppEvent::Instance()->slot_changeStyle();
    }

    QString logoImage = ui->cboxLogoImage->currentText().trimmed();
    if (AppConfig::LogoImage != logoImage) {
        AppConfig::LogoImage = logoImage;
        AppEvent::Instance()->slot_changeLogo();
        //立即设置全局图标
        if (AppConfig::LogoImage.startsWith("icon_")) {
            QString icon = AppConfig::LogoImage.split("_").last();
            GlobalConfig::IconMain = icon.toInt(NULL, 16);
        }
    }

    QString company = ui->txtCopyright->text().trimmed();
    if (AppConfig::Copyright != company) {
        AppConfig::Copyright = company;
        AppEvent::Instance()->slot_changeTitleInfo();
    }

    QString titleCn = ui->txtTitleCn->text().trimmed();
    if (AppConfig::TitleCn != titleCn) {
        AppConfig::TitleCn = titleCn;
        AppEvent::Instance()->slot_changeTitleInfo();
    }

    QString titleEn = ui->txtTitleEn->text().trimmed();
    if (AppConfig::TitleEn != titleEn) {
        AppConfig::TitleEn = titleEn;
        AppEvent::Instance()->slot_changeTitleInfo();
    }

    AppConfig::writeConfig();
}

void frmConfigSystem::initAppConfig2()
{
    QStringList listTime, listTimex;
    listTime << "0 秒钟" << "10 秒钟" << "20 秒钟" << "30 秒钟" << "1 分钟" << "2 分钟" << "5 分钟";
    listTimex << "0" << "10" << "20" << "30" << "60" << "120" << "300";

    int count = listTime.count();
    for (int i = 0; i < count; ++i) {
        ui->cboxTimeHideCursor->addItem(listTime.at(i), listTimex.at(i));
        ui->cboxTimeAutoFull->addItem(listTime.at(i), listTimex.at(i));
    }

    ui->cboxTimeHideCursor->setCurrentIndex(ui->cboxTimeHideCursor->findData(AppConfig::TimeHideCursor));
    ui->cboxTimeAutoFull->setCurrentIndex(ui->cboxTimeAutoFull->findData(AppConfig::TimeAutoFull));
    connect(ui->cboxTimeHideCursor, SIGNAL(currentIndexChanged(int)), this, SLOT(saveAppConfig2()));
    connect(ui->cboxTimeAutoFull, SIGNAL(currentIndexChanged(int)), this, SLOT(saveAppConfig2()));

    QStringList listCount, listCountx;
    listCount << "0 次" << "1 次" << "3 次" << "5 次" << "10 次" << "20 次" << "50 次" << "100 次";
    listCountx << "0" << "1" << "3" << "5" << "10" << "20" << "50" << "100";

    count = listCount.count();
    for (int i = 0; i < count; ++i) {
        ui->cboxAlarmSoundCount->addItem(listCount.at(i), listCountx.at(i));
    }

    ui->cboxAlarmSoundCount->setCurrentIndex(ui->cboxAlarmSoundCount->findData(AppConfig::AlarmSoundCount));
    connect(ui->cboxAlarmSoundCount, SIGNAL(currentIndexChanged(int)), this, SLOT(saveAppConfig2()));

    QStringList msgCount, msgCountx;
    msgCount << "0 行" << "1 行" << "3 行" << "5 行" << "10 行" << "15 行" << "20 行" << "25 行" << "30 行" << "50 行";
    msgCountx << "0" << "1" << "3" << "5" << "10" << "15" << "20" << "25" << "30" << "50";

    count = msgCount.count();
    for (int i = 0; i < count; ++i) {
        ui->cboxMsgListCount->addItem(msgCount.at(i), msgCountx.at(i));
        ui->cboxMsgTableCount->addItem(msgCount.at(i), msgCountx.at(i));
    }

    ui->cboxMsgListCount->setCurrentIndex(ui->cboxMsgListCount->findData(AppConfig::MsgListCount));
    ui->cboxMsgTableCount->setCurrentIndex(ui->cboxMsgTableCount->findData(AppConfig::MsgTableCount));
    connect(ui->cboxMsgListCount, SIGNAL(currentIndexChanged(int)), this, SLOT(saveAppConfig2()));
    connect(ui->cboxMsgTableCount, SIGNAL(currentIndexChanged(int)), this, SLOT(saveAppConfig2()));

    //0=一直显示 1000=禁用
    QStringList listTipInterval, listTipIntervalx;
    listTipInterval << "0 秒钟" << "3 秒钟" << "5 秒钟" << "10 秒钟" << "20 秒钟" << "30 秒钟" << "1 分钟" << "2 分钟" << "禁用";
    listTipIntervalx << "0" << "3" << "5" << "10" << "20" << "30" << "60" << "120" << "10000";

    count = listTipInterval.count();
    for (int i = 0; i < count; ++i) {
        ui->cboxTipInterval->addItem(listTipInterval.at(i), listTipIntervalx.at(i));
    }

    ui->cboxTipInterval->setCurrentIndex(ui->cboxTipInterval->findData(AppConfig::TipInterval));
    connect(ui->cboxTipInterval, SIGNAL(currentIndexChanged(int)), this, SLOT(saveAppConfig2()));

    ui->labWindowOpacityValue->setText(QString::number(AppConfig::WindowOpacity));
    ui->sliderWindowOpacity->setValue(AppConfig::WindowOpacity);
    ui->sliderWindowOpacity->setRange(10, 100);
    connect(ui->sliderWindowOpacity, SIGNAL(valueChanged(int)), this, SLOT(saveAppConfig2()));
}

void frmConfigSystem::saveAppConfig2()
{
    AppConfig::TimeHideCursor = ui->cboxTimeHideCursor->itemData(ui->cboxTimeHideCursor->currentIndex()).toInt();
    AppConfig::TimeAutoFull = ui->cboxTimeAutoFull->itemData(ui->cboxTimeAutoFull->currentIndex()).toInt();
    AppConfig::AlarmSoundCount = ui->cboxAlarmSoundCount->itemData(ui->cboxAlarmSoundCount->currentIndex()).toInt();
    AppConfig::MsgListCount = ui->cboxMsgListCount->itemData(ui->cboxMsgListCount->currentIndex()).toInt();
    AppConfig::MsgTableCount = ui->cboxMsgTableCount->itemData(ui->cboxMsgTableCount->currentIndex()).toInt();
    AppConfig::TipInterval = ui->cboxTipInterval->itemData(ui->cboxTipInterval->currentIndex()).toInt();

    int windowOpacity = ui->sliderWindowOpacity->value();
    if (AppConfig::WindowOpacity != windowOpacity) {
        AppConfig::WindowOpacity = windowOpacity;
        ui->labWindowOpacityValue->setText(QString::number(AppConfig::WindowOpacity));
        AppEvent::Instance()->slot_changeWindowOpacity();
    }

    AppConfig::writeConfig();
}

void frmConfigSystem::initAppConfig3()
{
    //从配置文件设置对应值到控件
    ui->btnUse96Dpi->setChecked(!AppConfig::Use96Dpi);
    ui->btnAutoRun->setChecked(AppConfig::AutoRun);
    ui->btnAutoLogin->setChecked(AppConfig::AutoLogin);
    ui->btnAutoPwd->setChecked(AppConfig::AutoPwd);
    ui->btnSaveLog->setChecked(AppConfig::SaveLog);
    ui->btnSaveRunTime->setChecked(AppConfig::SaveRunTime);
    ui->btnFullScreen->setChecked(AppConfig::FullScreen);
    ui->btnRightInfo->setChecked(AppConfig::RightInfo);
    ui->btnSaveUrl->setChecked(AppConfig::SaveUrl);

    //绑定变化事件自动保存配置文件
    connect(ui->btnUse96Dpi, SIGNAL(checkedChanged(bool)), this, SLOT(saveAppConfig3()));
    connect(ui->btnAutoRun, SIGNAL(checkedChanged(bool)), this, SLOT(saveAppConfig3()));
    connect(ui->btnAutoLogin, SIGNAL(checkedChanged(bool)), this, SLOT(saveAppConfig3()));
    connect(ui->btnAutoPwd, SIGNAL(checkedChanged(bool)), this, SLOT(saveAppConfig3()));
    connect(ui->btnSaveLog, SIGNAL(checkedChanged(bool)), this, SLOT(saveAppConfig3()));
    connect(ui->btnSaveRunTime, SIGNAL(checkedChanged(bool)), this, SLOT(saveAppConfig3()));
    connect(ui->btnFullScreen, SIGNAL(checkedChanged(bool)), this, SLOT(saveAppConfig3()));
    connect(ui->btnRightInfo, SIGNAL(checkedChanged(bool)), this, SLOT(saveAppConfig3()));
    connect(ui->btnSaveUrl, SIGNAL(checkedChanged(bool)), this, SLOT(saveAppConfig3()));

    //日志记录设置
    SaveLog::Instance()->setPath(QtHelper::appPath() + "/log");
    if (AppConfig::SaveLog) {
        SaveLog::Instance()->start();
    }

    //运行时间设置
    SaveRunTime::Instance()->setPath(QtHelper::appPath() + "/log");
    if (AppConfig::SaveRunTime) {
        SaveRunTime::Instance()->start();
    }
}

void frmConfigSystem::saveAppConfig3()
{
    bool use96Dpi = !ui->btnUse96Dpi->getChecked();
    if (AppConfig::Use96Dpi != use96Dpi) {
        AppConfig::Use96Dpi = use96Dpi;
        FormHelper::reboot(false);
    }

    AppConfig::AutoRun = ui->btnAutoRun->getChecked();
    AppConfig::AutoLogin = ui->btnAutoLogin->getChecked();
    AppConfig::AutoPwd = ui->btnAutoPwd->getChecked();
    AppConfig::FullScreen = ui->btnFullScreen->getChecked();

    //立即开启或者停止调试日志记录
    bool saveLog = ui->btnSaveLog->getChecked();
    if (AppConfig::SaveLog != saveLog) {
        AppConfig::SaveLog = saveLog;
        AppConfig::SaveLog ? SaveLog::Instance()->start() : SaveLog::Instance()->stop();
    }

    //立即开启或者停止运行时间记录
    bool saveRunTime = ui->btnSaveRunTime->getChecked();
    if (AppConfig::SaveRunTime != saveRunTime) {
        AppConfig::SaveRunTime = saveRunTime;
        AppConfig::SaveRunTime ? SaveRunTime::Instance()->start() : SaveRunTime::Instance()->stop();
    }

    bool rightInfo = ui->btnRightInfo->getChecked();
    if (AppConfig::RightInfo != rightInfo) {
        AppConfig::RightInfo = rightInfo;
        AppEvent::Instance()->slot_changeTitleInfo();
    }

    AppConfig::SaveUrl = ui->btnSaveUrl->getChecked();
    AppConfig::writeConfig();
}

void frmConfigSystem::initColorConfig()
{
    ui->cboxColorTextNormal->setColorName(AppConfig::ColorTextNormal);
    connect(ui->cboxColorTextNormal, SIGNAL(colorChanged(QString)), this, SLOT(saveColorConfig()));

    ui->cboxColorTextWarn->setColorName(AppConfig::ColorTextWarn);
    connect(ui->cboxColorTextWarn, SIGNAL(colorChanged(QString)), this, SLOT(saveColorConfig()));

    ui->cboxColorTextAlarm->setColorName(AppConfig::ColorTextAlarm);
    connect(ui->cboxColorTextAlarm, SIGNAL(colorChanged(QString)), this, SLOT(saveColorConfig()));

    ui->cboxColorIconAlarm->setColorName(AppConfig::ColorIconAlarm);
    connect(ui->cboxColorIconAlarm, SIGNAL(colorChanged(QString)), this, SLOT(saveColorConfig()));

    ui->cboxColorMsgWait->setColorName(AppConfig::ColorMsgWait);
    connect(ui->cboxColorMsgWait, SIGNAL(colorChanged(QString)), this, SLOT(saveColorConfig()));

    ui->cboxColorMsgSolved->setColorName(AppConfig::ColorMsgSolved);
    connect(ui->cboxColorMsgSolved, SIGNAL(colorChanged(QString)), this, SLOT(saveColorConfig()));
}

void frmConfigSystem::saveColorConfig()
{
    AppConfig::ColorTextNormal = ui->cboxColorTextNormal->getColorName();
    AppConfig::ColorTextWarn = ui->cboxColorTextWarn->getColorName();
    AppConfig::ColorTextAlarm = ui->cboxColorTextAlarm->getColorName();
    AppConfig::ColorIconAlarm = ui->cboxColorIconAlarm->getColorName();
    AppConfig::ColorMsgWait = ui->cboxColorMsgWait->getColorName();
    AppConfig::ColorMsgSolved = ui->cboxColorMsgSolved->getColorName();
    AppConfig::writeConfig();
}
