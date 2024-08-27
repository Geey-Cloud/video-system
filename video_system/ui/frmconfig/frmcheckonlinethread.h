#ifndef FRMCHECKONLINETHREAD_H
#define FRMCHECKONLINETHREAD_H

#include <QThread>
#include <QDebug>
#include <QTime>
#include <QTimer>

#ifndef TIMEMS
#define TIMEMS qPrintable(QTime::currentTime().toString("hh:mm:ss zzz"))
#endif

namespace Ui {
class frmCheckOnlineThread;
}

class frmCheckOnlineThread : public QThread
{
    Q_OBJECT
public:
    explicit frmCheckOnlineThread(QObject *parent = 0);
    ~frmCheckOnlineThread();

    void run() override;
    void stop();

    //设置检测时间间隔
    void setTimerInterval(const int &timerInterval);
signals:
    void devNetChanged(const QStringList &devOnlineInfo);

public slots:
    void onReceiveDevOnlineInfoSlot(const QStringList &devOnlineInfo);
    void delDevOnlineInfoSlot(const QStringList &devOnlineInfo, const bool &one);

private:
    QTimer *checkOnlineTimer;
    int timerInterval = 30000; // 默认30秒检测一次
    QList<QStringList> devOnlineInfos;

private slots:
    //检测设备上下线
    void checkOnline();
};

#endif // FRMCHECKONLINETHREAD_H
