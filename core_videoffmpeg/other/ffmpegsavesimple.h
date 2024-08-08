#ifndef FFMPEGSAVESIMPLE_H
#define FFMPEGSAVESIMPLE_H

#include "qthread.h"
#include "ffmpeghelper.h"

class FFmpegSaveSimple : public QThread
{
    Q_OBJECT
public:
    explicit FFmpegSaveSimple(QObject *parent = 0);
    ~FFmpegSaveSimple();

protected:
    void run();

private:
    //停止线程标志位
    volatile bool stopped;
    //是否是保存文件
    bool saveFile;

    //输入输出地址/可以是保存到本地或者推流
    QString urlIn;
    QString urlOut;

    //音视频索引
    int audioIndex;
    int videoIndex;

    //输入输出格式上下文
    AVFormatContext *formatCtxIn;
    AVFormatContext *formatCtxOut;

private:
    //打开输入输出
    bool openInput();
    bool openOutput();

    //关闭和释放
    void close();

public slots:
    //打印信息
    void debug(int ret, const QString &msg);
    //设置地址
    void setUrl(const QString &urlIn, const QString &urlOut);
    //停止线程
    void stop();
};

#endif // FFMPEGSAVESIMPLE_H
