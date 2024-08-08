#include "devicevideosmart.h"
#include "qthelper.h"
#include "appevent.h"
#include "videowidgetx.h"

DeviceVideoSmart::DeviceVideoSmart(VideoWidget *videoWidget, QObject *parent) : QThread(parent)
{
    stopped = false;
    isPause = false;
    this->videoWidget = videoWidget;

    //关联播放开始和完成信号
    connect(videoWidget, SIGNAL(sig_receivePlayStart(int)), this, SLOT(receivePlayStart(int)));
    connect(videoWidget, SIGNAL(sig_receivePlayFinsh()), this, SLOT(receivePlayFinsh()));
    //关联实时码率信号将实时码率作为文字水印贴到视频上
    connect(videoWidget, SIGNAL(sig_receiveKbps(qreal, int)), this, SLOT(receiveKbps(qreal, int)));

    //需要注册下这个类型
    qRegisterMetaType<QList<QRect> >("QList<QRect>");
}

//定义三个标志位控制启用哪些测试
static bool testImage = false;
static bool testGraph = true;
static bool testOsd = true;
static int sleepTime = 50;

void DeviceVideoSmart::run()
{
    while (!stopped) {
        if (isPause || AppConfig::Polling || !AppConfig::VideoSmart || !videoWidget || !videoWidget->getIsRunning()) {
            msleep(10);
            continue;
        }

        //定义区域集合
        QList<QRect> rects;
        if (testImage) {
            //获取一张图片用于人工智能处理
            QImage image = videoWidget->getImage();
            //qDebug() << TIMEMS << videoWidget->getVideoThread()->getFlag() << image.size();

            //人工智能处理算法放在这里进行运算
            //yolo->take(image);
        }

        //这里演示图形区域信息
        if (testGraph) {
            //限定下区域
            int w = videoWidget->getVideoWidth();
            int h = videoWidget->getVideoHeight();
            int width = w - 100;
            int height = h - 100;
            int wx = w / 50;
            int hx = h / 50;

            //随机生成矩形
            for (int i = 0; i < 30; ++i) {
                int x = rand() % width;
                int y = rand() % height;
                int w = wx + rand() % 40;
                int h = hx + rand() % 30;
                rects << QRect(x, y, w, h);
            }

            //异步执行
            QMetaObject::invokeMethod(this, "receiveRects", Q_ARG(QList<QRect>, rects));
        }

        //这里演示文字标签信息
        if (testOsd) {
            int currentCount = rand() % 60;
            int maxCount = rand() % 60 + 30;
            //qDebug() << TIMEMS << currentCount << currentThreadId();
            QMetaObject::invokeMethod(this, "receiveData", Q_ARG(QString, "currentCount"), Q_ARG(QVariant, currentCount));
            QMetaObject::invokeMethod(this, "receiveData", Q_ARG(QString, "maxCount"), Q_ARG(QVariant, maxCount));
        }

        //必须要有这个延时休息一下
        msleep(sleepTime);
    }

    stopped = false;
    isPause = false;
}

QColor DeviceVideoSmart::getColor()
{
    //取随机颜色
    static QStringList colors = QColor::colorNames();
    return colors.at(rand() % colors.count());
}

void DeviceVideoSmart::receivePlayStart(int time)
{
    //没开启对应功能则不用继续
    if (!AppConfig::VideoSmart) {
        return;
    }

    //字体大小按照视频宽度比例来
    fontSize = videoWidget->getVideoWidth() / 30;
    //设置时间水印
    this->receiveData("dateTime", 0);
    //设置启用视频帧数据锁
#ifdef ffmpeg
    QMetaObject::invokeMethod(videoWidget->getVideoThread(), "setLockData", Q_ARG(bool, true));
#endif

    //播放开始启动线程
    stopped = false;
    isPause = false;
    this->start();
}

void DeviceVideoSmart::receivePlayFinsh()
{
    //播放结束停止线程
    this->stop();
}

void DeviceVideoSmart::receiveKbps(qreal kbps, int frameRate)
{
    OsdInfo osd;
    osd.name = "kbps";
    osd.color = "#FF0000";
    osd.fontSize = fontSize;
    osd.position = OsdPosition_RightTop;
    osd.text = QString("%1 kbps\n%2 fps").arg((int)kbps).arg(frameRate);
    videoWidget->setOsd(osd);
}

void DeviceVideoSmart::receiveRects(const QList<QRect> &rects)
{
    int borderWidth = videoWidget->getVideoWidth() / 500;
    borderWidth = borderWidth < 1 ? 1 : borderWidth;

    int count = rects.count();
    for (int i = 0; i < count; ++i) {
        GraphInfo graph;
        graph.name = QString("graph%1").arg(i);
        graph.borderColor = getColor();
        graph.borderWidth = borderWidth;
        graph.rect = rects.at(i);
        videoWidget->setGraph(graph);
    }
}

void DeviceVideoSmart::receiveData(const QString &type, const QVariant &data)
{
    OsdInfo osd;
    osd.name = type;
    if (type == "dateTime") {
        osd.fontSize = fontSize;
        osd.color = "#FFFFFF";
        osd.format = OsdFormat_DateTime;
        osd.position = OsdPosition_LeftTop;
        videoWidget->setOsd(osd);
    } else if (type == "currentCount") {
        osd.fontSize = fontSize;
        osd.text = QString("当前人数: %1").arg(data.toInt());
        osd.color = getColor();//"#D64D54";
        osd.format = OsdFormat_Text;
        osd.position = OsdPosition_LeftBottom;
        videoWidget->setOsd(osd);
    } else if (type == "maxCount") {
        osd.fontSize = fontSize;
        osd.text = QString("限定人数: %1").arg(data.toInt());
        osd.color = getColor(); "#FDCD72";
        osd.format = OsdFormat_Text;
        osd.position = OsdPosition_RightBottom;
        videoWidget->setOsd(osd);
    }
}

void DeviceVideoSmart::stop()
{
    stopped = true;
    this->wait();
}

void DeviceVideoSmart::pause()
{
    isPause = true;
}

void DeviceVideoSmart::next()
{
    isPause = false;
}
