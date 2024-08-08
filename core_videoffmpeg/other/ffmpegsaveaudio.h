#ifndef FFMPEGSAVEAUDIO_H
#define FFMPEGSAVEAUDIO_H

#include "qthread.h"
#include "ffmpeghelper.h"

class FFmpegSaveAudio : public QThread
{
    Q_OBJECT
public:
    explicit FFmpegSaveAudio(QObject *parent = 0);
    ~FFmpegSaveAudio();

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

    //音频索引
    int audioIndex;
    //音频时长
    qint64 duration;

    //输入输出格式上下文
    AVFormatContext *formatCtxIn;
    AVFormatContext *formatCtxOut;

    //输出输出音频流
    AVStream *streamIn;
    AVStream *streamOut;

    //编解码器上下文/输入用于解码/输出用于编码
    AVCodecContext *codecCtxIn;
    AVCodecContext *codecCtxOut;

    //输入输出数据包
    AVPacket *packetIn;
    AVPacket *packetOut;

    //输入输出数据帧
    AVFrame *frameIn;
    AVFrame *frameOut;

    //音频缓冲
    AVAudioFifo *audioFifo;

private:
    //打开输入输出
    bool openInput();
    bool openOutput();

    //关闭和释放
    void close();

    //解码音频数据到pcm
    void decode(AVPacket *packet);
    //编码音频数据到aac
    void encode(AVFrame *frame);

public slots:
    //打印信息
    void debug(int ret, const QString &msg);
    //设置地址
    void setUrl(const QString &urlIn, const QString &urlOut);
    //停止线程
    void stop();
};

#endif // FFMPEGSAVEAUDIO_H
