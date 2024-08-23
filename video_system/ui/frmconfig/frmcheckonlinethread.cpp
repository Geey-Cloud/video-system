#include "frmcheckonlinethread.h"
#include "devicehelper.h"

frmCheckOnlineThread::frmCheckOnlineThread(QObject *parent)
    : QThread(parent)
{

}

frmCheckOnlineThread::~frmCheckOnlineThread()
{
    checkOnlineTimer->stop();
}

void frmCheckOnlineThread::run()
{
    checkOnlineTimer = new QTimer;
    checkOnlineTimer->setInterval(timerInterval);
    connect(checkOnlineTimer, SIGNAL(timeout()), this, SLOT(checkOnline()));
    checkOnlineTimer->start();
    this->exec();
}

void frmCheckOnlineThread::stop()
{
    checkOnlineTimer->stop();
}

void frmCheckOnlineThread::checkOnline()
{
    // qDebug() << TIMEMS << QString("正在检测%1个设备的网络情况").arg(devOnlineInfos.count());
    for (int index = 0; index < devOnlineInfos.count(); ++index) {
        QStringList devOnlineInfo = devOnlineInfos.at(index);
        QString isOnline = DeviceHelper::checkDeviceOnline2(devOnlineInfo) ? "在线" : "离线";
        if (isOnline != devOnlineInfo.at(2)) {
            devOnlineInfos[index].removeAt(2);
            devOnlineInfos[index] << isOnline;
            emit devNetChanged(devOnlineInfos[index]);
        }
    }
}

void frmCheckOnlineThread::setTimerInterval(const int &timerInterval)
{
    this->timerInterval = timerInterval;
}

void frmCheckOnlineThread::onReceiveDevOnlineInfoSlot(const QStringList &devOnlineInfo)
{
    if (3 == devOnlineInfo.count()) {
        this->devOnlineInfos.append(devOnlineInfo);
    } else {
        qDebug() << TIMEMS << "设备网络信息长度不正确" << devOnlineInfo;
    }
}

void frmCheckOnlineThread::delDevOnlineInfoSlot(const QStringList &devOnlineInfo, const bool &one)
{
    if (!one) {
        devOnlineInfos.clear();
        return;
    }

    for (auto it = devOnlineInfos.begin(); it != devOnlineInfos.end(); ++it) {
        if (*it == devOnlineInfo){
            devOnlineInfos.erase(it);
            // qDebug() << TIMEMS << QString("目前正在判断%1个设备的上下线情况").arg(this->devOnlineInfos.count());
            return;
        }
    }
}


