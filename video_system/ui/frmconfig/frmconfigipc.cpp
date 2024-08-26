#include "frmconfigipc.h"
#include "ui_frmconfigipc.h"
#include "qthelper.h"
#include "devicehelper.h"
#include "urlhelper.h"
#include "dbquery.h"
#include "dbdelegate.h"
#include "commonkey.h"
#include "frmconfigplus.h"

frmConfigIpc::frmConfigIpc(QWidget *parent) : QWidget(parent), ui(new Ui::frmConfigIpc)
{
    ui->setupUi(this);
    this->initForm();
    this->initData();
    this->initIcon();
}

frmConfigIpc::~frmConfigIpc()
{
    checkOnlineThread->stop();
    delete ui;
}

void frmConfigIpc::showEvent(QShowEvent *)
{
    model->select();
}

void frmConfigIpc::initForm()
{
    ui->widgetTop->setProperty("flag", "navbtn");
    ui->labTip->setText("提示 → 改动后立即应用");

    // todo 修复点击会超界的bug
    ui->btnAdd->setEnabled(false);
    ui->btnAdd->hide();
    ui->btnPlus->setEnabled(false);
    ui->btnPlus->hide();

    //初始化通用的表格属性
    QtHelper::initTableView(ui->tableView, AppData::RowHeight, false, true);
    //关联样式改变信号自动重新设置图标
    connect(AppEvent::Instance(), SIGNAL(changeStyle()), this, SLOT(initIcon()));

    if (AppConfig::TableDataPolicy == 1) {
        //设置支持多选
        ui->tableView->setSelectionMode(QAbstractItemView::MultiSelection);
        //双击进入编辑
        ui->tableView->setEditTriggers(QAbstractItemView::DoubleClicked);
    }

    //显示面板以及关联对应的信号槽
    ui->widgetIpcSearch->setVisible(AppConfig::VisibleIpcSearch);
    connect(ui->widgetIpcSearch, SIGNAL(addDevices(QList<QStringList>)), this, SLOT(addDevices(QList<QStringList>)));

    //关联批量添加窗体信号
    connect(frmConfigPlus::Instance(), SIGNAL(addPlus(QStringList, QStringList)), this, SLOT(addPlus(QStringList, QStringList)));

    //设备上下线线程以及关联对应的信号槽
    checkOnlineThread = new frmCheckOnlineThread();
    connect(this, SIGNAL(devOnlineInfoSigal(QStringList)), checkOnlineThread, SLOT(onReceiveDevOnlineInfoSlot(QStringList)));
    connect(this, SIGNAL(devOnlineInfoDelSignal(QStringList,bool)), checkOnlineThread, SLOT(delDevOnlineInfoSlot(QStringList, bool)));
    connect(checkOnlineThread, SIGNAL(devNetChanged(QStringList)), this, SLOT(ipcNetChanged(QStringList)));
}

void frmConfigIpc::initIcon()
{
    //设置按钮图标
    CommonNav::setIconBtn(ui->widgetTop);
}

void frmConfigIpc::initData()
{
    //实例化数据库表模型
    model = new QSqlTableModel(this);
    //绑定数据库表到数据模型
    DbHelper::bindTable(AppConfig::LocalDbType, model, "IpcInfo");
    //设置选取数据条件
    // model->setQuery("SELECT IpcID, IpcName, NvrName, IpcType, OnvifAddr, ProfileToken, VideoSource, RtspMain, "
    //                 "IpcPosition, IpcImage, IpcX, IpcY, UserName, UserPwd, IpcEnable, IpcMark FROM IpcInfo;");
    //设置过滤条件
    //model->setFilter("IpcType='http' and IpcID='5'");
    //设置列排序
    model->setSort(0, Qt::AscendingOrder);
    //设置提交模式
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    //立即查询一次
    model->select();
    //将数据库表模型设置到表格上
    ui->tableView->setModel(model);

    //初始化列名和列宽
    columnNames << "编号" << "名称" << "录像机" << "厂家" << "设备地址" << "配置文件" << "视频文件" << "主码流地址" << "子码流地址" //8
                << "经纬度" << "地图" << "X坐标" << "Y坐标" << "用户姓名" << "用户密码" << "启用" //15
                << "IP地址" << "管理端口" << "设备型号" << "固件版本" << "序列号" << "是否在线" << "操作"; //22
    columnWidths << 40 << 160 << 90 << 80 << 250 << 100 << 100 << 130 << 130
                 << 150 << 90 << 45 << 45 << 80 << 80 << 40
                 << 120 << 100 << 220 << 320 << 320 << 80 << 120;

    // //特殊分辨率重新设置列宽
    int count = columnNames.count();
    // if (QtHelper::deskWidth() >= 1920) {
    //     for (int i = 0; i < count - 2; ++i) {
    //         columnWidths[i] += 30;
    //     }

    //     columnWidths[7] = 350;
    //     columnWidths[8] = 350;
    // }

    //挨个设置列名和列宽
    for (int i = 0; i < count; ++i) {
        model->setHeaderData(i, Qt::Horizontal, columnNames.at(i));
        ui->tableView->setColumnWidth(i, columnWidths.at(i));
    }


    //设置隐藏列
    ui->tableView->setColumnHidden(2, true);
    ui->tableView->setColumnHidden(4, true);
    ui->tableView->setColumnHidden(5, true);
    ui->tableView->setColumnHidden(6, true);
    ui->tableView->setColumnHidden(7, true);
    ui->tableView->setColumnHidden(8, true);
    ui->tableView->setColumnHidden(9, true);
    ui->tableView->setColumnHidden(10, true);
    ui->tableView->setColumnHidden(11, true);
    ui->tableView->setColumnHidden(12, true);
    ui->tableView->setColumnHidden(13, true);
    ui->tableView->setColumnHidden(14, true);
    ui->tableView->setColumnHidden(15, true);
// #if 1 // 不显示子码流地址
//     ui->tableView->setColumnHidden(8, true);
//     ui->tableView->setColumnWidth (7, ui->tableView->columnWidth(7) * 2);
// #endif
//     ui->tableView->setColumnHidden(11, true);
//     ui->tableView->setColumnHidden(12, true);

    // //录像机委托
    // d_cbox_nvrName = new DbDelegate(this);
    // d_cbox_nvrName->setDelegateType("QComboBox");
    // ui->tableView->setItemDelegateForColumn(2, d_cbox_nvrName);
    // nvrNameChanged();


    // //类型委托
    // DbDelegate *d_cbox_ipcType = new DbDelegate(this);
    // d_cbox_ipcType->setDelegateType("QComboBox");
    // d_cbox_ipcType->setDelegateValue(AppData::IpcTypes);
    // // ui->tableView->setItemDelegateForColumn(3, d_cbox_ipcType);

    // //背景地图委托
    // d_cbox_ipcImage = new DbDelegate(this);
    // d_cbox_ipcImage->setDelegateType("QComboBox");
    // ui->tableView->setItemDelegateForColumn(10, d_cbox_ipcImage);
    // ipcImageChanged();

    // //用户密码委托
    // DbDelegate *d_txt_userPwd = new DbDelegate(this);
    // d_txt_userPwd->setDelegateType("QLineEdit");
    // d_txt_userPwd->setDelegatePwd(true);
    // d_txt_userPwd->setDelegateColumn(14);
    // ui->tableView->setItemDelegateForColumn(14, d_txt_userPwd);

    // //启用禁用委托
    // DbDelegate *d_ckbox_ipcEnable = new DbDelegate(this);
    // d_ckbox_ipcEnable->setDelegateColumn(15);
    // d_ckbox_ipcEnable->setDelegateType("QCheckBox");
    // d_ckbox_ipcEnable->setCheckBoxText("启用", "禁用");
    // ui->tableView->setItemDelegateForColumn(15, d_ckbox_ipcEnable);

    if (!checkOnlineThread->isRunning()) {
        checkOnlineThread->start();

        QStringList devOnlineInfo;
        //将表格中每一行的数据发送到设备离线检测线程中判断
        for (int row = 0; row < ui->tableView->model()->rowCount(); ++row) {
            QString ip = ui->tableView->model()->data(ui->tableView->model()->index(row, 16)).toString();
            QString port = ui->tableView->model()->data(ui->tableView->model()->index(row, 17)).toString();
            QString isOnline = ui->tableView->model()->data(ui->tableView->model()->index(row, 21)).toString();
            devOnlineInfo.clear();
            devOnlineInfo << ip << port << isOnline;
            emit devOnlineInfoSigal(devOnlineInfo);
        }
    }
}

void frmConfigIpc::nvrNameChanged()
{
    QStringList nvrNames;
    foreach (QString nvrName, DbData::NvrInfo_NvrName) {
        if (!nvrNames.contains(nvrName)) {
            nvrNames << nvrName;
        }
    }

    d_cbox_nvrName->setDelegateValue(nvrNames);
}

void frmConfigIpc::ipcImageChanged()
{
    QStringList mapNames;
    mapNames << " -- 无 -- " << AppData::MapNames;
    d_cbox_ipcImage->setDelegateValue(mapNames);
}

void frmConfigIpc::ipcNetChanged(const QStringList &devOnlineInfo)
{
    qDebug() << TIMEMS << devOnlineInfo;
    QString ip = devOnlineInfo.at(0);
    QString netState = devOnlineInfo.at(2);
    for (int row = 0; row < model->rowCount(); ++row) {
        QString ip_model = model->data(model->index(row, 16)).toString();
        //找到网络改变的设备所在行
        if (ip == ip_model) {
            model->setData(model->index(row, 21), netState);
            DeviceHelper::setVideoIcon2(ip, netState == "离线" ? true : false);
            break;
        }
    }
}

void frmConfigIpc::addDevice(const QStringList &deviceInfo)
{
    // for (int index = 0; index < deviceInfo.count(); ++index){
    //     qDebug() << index << deviceInfo[index];
    // }

    //插入一行
    int count = model->rowCount();
    model->insertRow(count);

    //获取上一行的对应列的数据
    int row = count - 1;
    int ipcID = model->index(row, 0).data().toInt() + 1;
    QString ipcName = model->index(row, 1).data().toString();
    QString nvrName = model->index(row, 2).data().toString();
    QString ipcType = model->index(row, 3).data().toString();
    QString onvifAddr = model->index(row, 4).data().toString();
    QString profileToken = model->index(row, 5).data().toString();
    QString videoSource = model->index(row, 6).data().toString();
    QString rtspMain = model->index(row, 7).data().toString();
    QString rtspSub = model->index(row, 8).data().toString();
    QString ipcPosition = model->index(row, 9).data().toString();
    QString ipcImage = model->index(row, 10).data().toString();
    int ipcX = model->index(row, 11).data().toInt();
    int ipcY = model->index(row, 12).data().toInt();
    QString userName = model->index(row, 13).data().toString();
    QString userPwd = model->index(row, 14).data().toString();
    QString ipcEnable = model->index(row, 15).data().toString();
    QString ip = model->index(row, 16).data().toString();
    int port = model->index(row, 17).data().toInt();
    QString deviceModel = model->index(row, 18).data().toString();
    QString firmwareVersion = model->index(row, 19).data().toString();
    QString SerialNumber = model->index(row, 20).data().toString();
    QString isOnline = DeviceHelper::checkDeviceOnline(deviceInfo) ? "在线" : "离线";

    //设备名称自定义递增规则 #后面紧跟序号
    int index = ipcName.indexOf("#");
    if (index >= 0) {
        int len = ipcName.length();
        // int number = ipcName.mid(index + 1, len).toInt();
        QStringList ips = ipcName.mid(index + 1, len).split(".");
        QString number = QString("%1").arg(ips.last().toInt() + 1);
        ips.removeLast();
        ips.append(number);
        QString ip = ips.join("_");
        QString flag = ipcName.mid(0, index);
        ipcName = QString("%1#%2").arg(flag).arg(ip);
    }

    //码流地址自定义递增规则(目前限定本地文件)
    index = rtspMain.indexOf("/mp4/push/");
    if (index >= 0) {
        int len = rtspMain.length();
        QString end = rtspMain.mid(index + 5, len);
        QStringList list = end.split(".");
        int number = list.first().toInt();
        QString flag = rtspMain.mid(0, index);
        rtspMain = QString("%1/mp4/push/%2.%3").arg(flag).arg(number + 1).arg(list.last());
        rtspSub = rtspMain;
    }

    //设备在图片上的位置自动排列
    FormHelper::checkPosition(ipcX, ipcY, AppData::DeviceWidth, AppData::DeviceHeight);

    //当前为第一行时候的默认值
    if (count == 0) {
        ipcID = 1;
        ipcName = AppConfig::DefaultIpcName;
        //默认取第一台NVR
        nvrName = DbData::NvrInfo_NvrName.first();
        nvrName = nvrName.isEmpty() ? AppConfig::DefaultNvrName : nvrName;
        ipcType = "other";

        rtspMain = "rtsp://192.168.1.128:554/0";
        rtspSub = "rtsp://192.168.1.128:554/1";

        //默认值取中心点
        ipcPosition = AppConfig::MapCenter;
        ipcPosition.replace(",", "|");

        ipcImage = AppData::MapNames.count() > 0 ? AppData::MapNames.first() : " -- 无 -- ";
        ipcX = 5;
        ipcY = 5;
        userName = "admin";
        userPwd = "admin";
        ipcEnable = "启用";
    }

    //如果是批量添加过来的数据
    if (deviceInfo.count() > 7) {
        userName = deviceInfo.at(0);
        userPwd = deviceInfo.at(1);
        ipcType = deviceInfo.at(2);
        onvifAddr = deviceInfo.at(3);
        profileToken = deviceInfo.at(4);
        videoSource = deviceInfo.at(5);
        rtspMain = deviceInfo.at(6);
        rtspSub = deviceInfo.at(7);
        ip = deviceInfo.at(8);
        port = deviceInfo.at(9).toInt();
        deviceModel = deviceInfo.at(10);
        firmwareVersion = deviceInfo.at(11);
        SerialNumber = deviceInfo.at(12);

        //重新定义搜索的摄像机设备命名规则,按照摄像机#ip地址末尾数字的方式
        QString ip = UrlHelper::getUrlIP(rtspMain);
        QStringList ips = ip.split(".");
        // QString flag = ips.last();
        QString flag = ips.join("_");
        ipcName = !onvifAddr.isEmpty() ? QString("摄像机#%1").arg(flag) : QString("视频流#%1").arg(flag);
        if (rtspMain.startsWith("video=") || rtspMain.startsWith("audio=") || rtspMain.startsWith("screen=")) {
            ipcName = QString("本地设备#%1").arg(ipcID);
        } else if (flag.isEmpty()) {
            ipcName = QString("视频文件#%1").arg(ipcID);
        }

        //NVR过来的再区分对应的通道 NVR_Ch1_Dahua
        if (ipcType.startsWith("NVR_Ch")) {
            //编号改成对应NVR地址的整型和3位通道组合(跨网段的现场一般是后两个网段变化)
            QString ch = ipcType.split("_").at(1);
            QString channel = QString("%1").arg(ch.mid(2, 3).toInt(), 3, 10, QChar('0'));
            int addr = (ips.at(3).toInt() | ips.at(2).toInt() << 8 | 0 << 16 | 0 << 24);
            QString id = QString("%1%2").arg(addr).arg(channel);
            ipcID = id.toInt();
            //ipcName = ipcName + "_" + ch;
            ipcName = QString("通道%1").arg(channel);

            //查找对应的NVR名称
            int index = DbData::NvrInfo_NvrIP.indexOf(ip);
            if (index >= 0) {
                nvrName = DbData::NvrInfo_NvrName.at(index);
            } else {
                //查找不到就主动添加这个NVR
                nvrName = QString("录像机#%1").arg(flag);
                QString nvrType = "其他";
                if (ipcType.endsWith("HIKVISION")) {
                    nvrType = "海康";
                } else if (ipcType.endsWith("Dahua")) {
                    nvrType = "大华";
                }

                DbQuery::addNvrInfo(ip, nvrName, nvrType);
                //添加完成后还要立即重新读取下不然会重复添加
                DbQuery::loadNvrInfo();
            }
        }
    }

    //设置新增加的行默认值
    model->setData(model->index(count, 0), ipcID);
    model->setData(model->index(count, 1), ipcName);
    model->setData(model->index(count, 2), nvrName);
    model->setData(model->index(count, 3), ipcType);
    model->setData(model->index(count, 4), onvifAddr);
    model->setData(model->index(count, 5), profileToken);
    model->setData(model->index(count, 6), videoSource);
    model->setData(model->index(count, 7), rtspMain);
    model->setData(model->index(count, 8), rtspSub);
    model->setData(model->index(count, 9), ipcPosition);
    model->setData(model->index(count, 10), ipcImage);
    model->setData(model->index(count, 11), ipcX);
    model->setData(model->index(count, 12), ipcY);
    model->setData(model->index(count, 13), userName);
    model->setData(model->index(count, 14), userPwd);
    model->setData(model->index(count, 15), ipcEnable);
    model->setData(model->index(count, 16), ip);
    model->setData(model->index(count, 17), port);
    model->setData(model->index(count, 18), deviceModel);
    model->setData(model->index(count, 19), firmwareVersion);
    model->setData(model->index(count, 20), SerialNumber);
    model->setData(model->index(count, 21), isOnline);

    QStringList devOnlineInfo;
    devOnlineInfo << ip << QString("%1").arg(port) << isOnline;
    emit devOnlineInfoSigal(devOnlineInfo);
}

void frmConfigIpc::addDevices(const QList<QStringList> &deviceInfos)
{
    //校验是否超过秘钥的数量限制
    //总数量=当前行数+将要添加的设备数量
    int count = model->rowCount() + deviceInfos.count() - 1;
    if (!CommonKey::checkCount(count)) {
        return;
    }

    count = deviceInfos.count();
    for (int i = 0; i < count; ++i) {
        QStringList deviceInfo = deviceInfos.at(i);
        QString onvifAddr = deviceInfo.at(3);
        QString rtspMain = deviceInfo.at(6);

        //过滤已经添加过的设备
        bool exist = false;
        for (int j = 0; j < DbData::IpcInfo_Count; ++j) {
            QString addr = DbData::IpcInfo_OnvifAddr.at(j);
            QString rtsp = DbData::IpcInfo_RtspMain.at(j);
            if (addr == onvifAddr && rtsp == rtspMain) {
                exist = true;
                break;
            }
        }

        if (!exist) {
            addDevice(deviceInfo);
        }
    }

    //单击保存按钮
    on_btnSave_clicked();
}

void frmConfigIpc::addPlus(const QStringList &rtspMains, const QStringList &rtspSubs)
{
    int count = rtspMains.count();
    if (count == 0 || !this->isVisible()) {
        return;
    }

    for (int i = 0; i < count; ++i) {
        //模拟成搜索那边批量添加的形式
        QString rtspMain = rtspMains.at(i);
        QString rtspSub = rtspSubs.at(i);
        QString userName = "admin";
        QString userPwd = "12345";

        int index = rtspMain.lastIndexOf("@");
        if (index > 0) {
            QString userInfo = rtspMain.mid(0, index);
            userInfo.replace("rtsp://", "");
            QStringList list = userInfo.split(":");
            userName = list.at(0);
            userPwd = list.at(1);
        }

        QString ipcType = "other";
        QString ip = UrlHelper::getUrlIP(rtspMain);
        QString onvifAddr = "";
        QString profileToken = "";
        QString videoSource = "";

        //如果过来的是本地设备则不需要取出各种信息
        if (!rtspMain.startsWith("video=") && !rtspMain.startsWith("audio=") && !rtspMain.startsWith("screen=")) {
            if (AppConfig::PlusType == 1) {
                ipcType = "HIKVISION";
                onvifAddr = QString("http://%1/onvif/device_service").arg(ip);
                profileToken = "Profile_1";
                videoSource = "VideoSource_1";
            } else if (AppConfig::PlusType == 2) {
                ipcType = "Dahua";
                onvifAddr = QString("http://%1/onvif/device_service").arg(ip);
                profileToken = "Profile000";
                videoSource = "VideoSource000";
            }

            //通道递增一般是NVR
            if (AppConfig::PlusType != 0 && AppConfig::PlusNvr) {
                ipcType = QString("NVR_Ch%1_%2").arg(i + 1).arg(ipcType);
                if (AppConfig::PlusType == 1) {
                    QString flag = QString("%1").arg(i + 1, 3, 10, QChar('0'));
                    profileToken = "ProfileToken" + flag;
                    videoSource = "VideoSourceToken" + flag;
                } else if (AppConfig::PlusType == 2) {
                    QString flag = QString("%100").arg(i, 3, 10, QChar('0'));
                    profileToken = "MediaProfile" + flag;
                    videoSource = flag;
                }
            }
        }

        QStringList deviceInfo;
        deviceInfo << userName << userPwd << ipcType << onvifAddr << profileToken << videoSource << rtspMain << rtspSub;
        addDevice(deviceInfo);
    }

    //单击保存按钮
    on_btnSave_clicked();
}

void frmConfigIpc::on_btnAdd_clicked()
{
    //校验是否超过秘钥的数量限制
    int count = model->rowCount();
    if (!CommonKey::checkCount(count)) {
        return;
    }

    //调用批量添加设备
    addDevice(QStringList());
    //立即选中当前新增加的行
    ui->tableView->setCurrentIndex(model->index(count, 0));
}

void frmConfigIpc::on_btnSave_clicked()
{
    //重新设置下焦点避免mac系统上当单元格处于编辑状态保存不成功的bug
    ui->tableView->setFocus();

    //开启数据库事务提交数据
    model->database().transaction();
    if (model->submitAll()) {
        model->database().commit();
        AppEvent::Instance()->slot_saveIpcInfo(true);
    } else {
        //提交失败则回滚事务并打印错误信息
        model->database().rollback();
        qDebug() << TIMEMS << model->database().lastError();
        QtHelper::showMessageBoxError("保存信息失败, 请重新填写!");
    }

    //有些数据库需要主动查询一下不然是空白的比如odbc数据源
    model->select();
}

void frmConfigIpc::on_btnDelete_clicked()
{
    int row = ui->tableView->currentIndex().row();
    if (row < 0) {
        QtHelper::showMessageBoxError("请选择要删除的行!");
        return;
    }

    if (QtHelper::showMessageBoxQuestion("确定要删除选中的摄像机吗?\n摄像机对应的轮询信息都会被删除!") == QMessageBox::Yes) {
        //获取选中行的内容
        QStringList ids, addrs, devOnlineInfoDel;
        QItemSelectionModel *selections = ui->tableView->selectionModel();
        QModelIndexList selected = selections->selectedIndexes();
        foreach (QModelIndex index, selected) {
            int column = index.column();
            QString text = index.data().toString();
            if (column == 0) {
                ids << text;
            } else if (column == 7) {
                addrs << text;
            } else if (column == 16 || column == 17 || column == 21) {
                devOnlineInfoDel <<text;
            }
        }

        DbQuery::deleteIpcInfos(ids.join(","));
        DbQuery::deletePollInfos(addrs.join(","));
        AppEvent::Instance()->slot_saveIpcInfo(true);
        model->select();
        ui->tableView->setCurrentIndex(model->index(model->rowCount() - 1, 0));

        emit devOnlineInfoDelSignal(devOnlineInfoDel, true);
    }
}

void frmConfigIpc::on_btnReturn_clicked()
{
    model->revertAll();
}

void frmConfigIpc::on_btnClear_clicked()
{
    if (model->rowCount() <= 0) {
        return;
    }

    if (QtHelper::showMessageBoxQuestion("确定要清空所有信息吗? 清空后不能恢复!") == QMessageBox::Yes) {
        DbQuery::clearIpcInfo();
        AppEvent::Instance()->slot_saveIpcInfo(true);
        model->select();

        QStringList nullString;
        emit devOnlineInfoDelSignal(nullString, false);
    }
}

void frmConfigIpc::on_btnInput_clicked()
{
    FormHelper::inputData(model, columnNames, "IpcInfo", "摄像机信息");
    on_btnSave_clicked();
}

void frmConfigIpc::on_btnOutput_clicked()
{
    FormHelper::outputData("IpcID asc", columnNames, "IpcInfo", "摄像机信息");
}

void frmConfigIpc::dataout(quint8 type)
{
    QList<QString> columnNames;
    QList<int> columnWidths;
    columnNames << "编号" << "名称" << "录像机" << "厂家" << "启用" << "备注";
    columnWidths << 50 << 120 << 120 << 150 << 60 << 100;

    QString columns = "IpcID,IpcName,NvrName,IpcType,IpcEnable,IpcMark";
    QString where = "order by IpcID asc";
    FormHelper::dataout(type, columnNames, columnWidths, "摄像机信息", "IpcInfo", columns, where);
}

void frmConfigIpc::on_btnPrint_clicked()
{
    dataout(0);
}

void frmConfigIpc::on_btnXls_clicked()
{
    dataout(2);
}

void frmConfigIpc::on_btnPlus_clicked()
{
    QWidget *widget = frmConfigPlus::Instance();
    if (!widget->isVisible()) {
        widget->show();
        QtHelper::setFormInCenter(widget);
    }
}

void frmConfigIpc::on_btnSearch_clicked()
{
    ui->widgetIpcSearch->setVisible(!ui->widgetIpcSearch->isVisible());
    AppConfig::VisibleIpcSearch = ui->widgetIpcSearch->isVisible();
    AppConfig::writeConfig();
}

void frmConfigIpc::on_btnAddRtspStream_clicked()
{
    QString inputResult = QtHelper::showInputBox("添加Rtsp流", 0, 0, "rtsp://admin:admin@192.168.0.1:554/LiveStream/CH0/MainStream/OnvifGet", false, "rtsp://admin:admin@192.168.0.1:554/LiveStream/CH0/MainStream/OnvifGet");
    if (inputResult.isEmpty()) {
        QtHelper::showMessageBoxInfo("请输入一个Rtsp流的地址");
        return;
    }
    if (!inputResult.startsWith("rtsp")) {
        QtHelper::showMessageBoxInfo("请输入一个Rtsp流的地址");
        return;
    }

    //todo 判断是不是Bova盒子的视频流
    QStringList resultList = inputResult.split("/");
    QString streamInfo = resultList.at(2);
    QStringList ipcInfoList = streamInfo.split("@");

    QString userName, userPwd, ip, port;
    if (1 == ipcInfoList.count()) { //不含用户名和密码
        QStringList addrInfo = ipcInfoList.at(0).split(":");
        userName = "";
        userPwd = "";
        ip = addrInfo.at(0);
        port = addrInfo.count() == 1 ? "554" : addrInfo.at(1);
    } else {
        QStringList ipcUserInfoList = ipcInfoList.at(0).split(":");
        QStringList addrInfo = ipcInfoList.at(1).split(":");

        userName = ipcUserInfoList.at(0);
        userPwd = ipcUserInfoList.at(1);
        ip = addrInfo.at(0);
        port = addrInfo.count() == 1 ? "554" : addrInfo.at(1);
    }

    //创建视频流的数据库信息
    QString company = "";
    QString ipcType = "";
    QString onvifAddr = "";
    QString profileToken = "";
    QString videoSource = "";
    QString rtspMain = inputResult;
    QString rtspSub = inputResult;
    QString model = "";
    QString firmwareVersion = "";
    QString serialNumber = "";

    QStringList deviceInfo;
    deviceInfo << userName << userPwd << company << onvifAddr << profileToken << videoSource << rtspMain << rtspSub
               << ip << port << model << firmwareVersion << serialNumber;
    emit addDevice(deviceInfo);
    //单击保存按钮
    on_btnSave_clicked();
}

