#include "ffmpegsavesimple.h"
#include "ffmpegsavehelper.h"

//用法示例(保存文件/推流)
#if 0
FFmpegSaveSimple *f = new FFmpegSaveSimple(this);
f->setUrl("f:/mp4/push/1.mp4", "f:/1.mp4");
f->setUrl("f:/mp4/push/1.mp4", "rtmp://127.0.0.1/stream");
f->start();
#endif

FFmpegSaveSimple::FFmpegSaveSimple(QObject *parent) : QThread(parent)
{
    stopped = false;
    saveFile = false;

    audioIndex = -1;
    videoIndex = -1;

    formatCtxIn = NULL;
    formatCtxOut = NULL;

    //初始化ffmpeg的库
    FFmpegHelper::initLib();
}

FFmpegSaveSimple::~FFmpegSaveSimple()
{
    this->stop();
    this->close();
}

void FFmpegSaveSimple::run()
{
    if (!this->openInput() || !this->openOutput()) {
        this->close();
        return;
    }

    int ret;
    AVPacket packet;
    qint64 videoCount = 0;
    qint64 startTime = av_gettime();

    while (!stopped) {
        //读取一帧
        int index = packet.stream_index;
        if ((ret = av_read_frame(formatCtxIn, &packet)) < 0) {
            if (ret == AVERROR_EOF || ret == AVERROR_EXIT) {
                debug(0, "文件结束");
                break;
            } else {
                debug(ret, "读取出错");
                continue;
            }
        }

        //取出输入输出流的时间基
        AVStream *streamIn = formatCtxIn->streams[index];
        AVStream *streamOut = formatCtxOut->streams[index];
        AVRational timeBaseIn = streamIn->time_base;
        AVRational timeBaseOut = streamOut->time_base;

        if (index == videoIndex) {
            videoCount++;
        } else if (index == audioIndex) {

        }

        //纠正有些文件比如h264格式的没有pts
        if (packet.pts == AV_NOPTS_VALUE) {
            qreal fps = av_q2d(formatCtxIn->streams[videoIndex]->r_frame_rate);
            FFmpegSaveHelper::rescalePacket(&packet, timeBaseIn, videoCount, fps);
        }

        //推流需要延时/防止数据过大撑爆缓存
        if (!saveFile && index == videoIndex) {
            FFmpegHelper::delayTime(formatCtxIn, &packet, startTime);
        }

        //重新调整时间基准
        FFmpegSaveHelper::rescalePacket(&packet, timeBaseIn, timeBaseOut);

        qDebug() << TIMEMS << "发送一帧" << index << videoCount << packet.flags << packet.pts << packet.dts;
        if ((ret = av_interleaved_write_frame(formatCtxOut, &packet)) < 0) {
            debug(ret, "写数据包");
            break;
        }

        av_packet_unref(&packet);
    }

    //写文件尾
    if ((ret = av_write_trailer(formatCtxOut)) < 0) {
        debug(ret, "写文件尾");
    }

    this->close();
}

bool FFmpegSaveSimple::openInput()
{
    int ret = -1;
    if ((ret = avformat_open_input(&formatCtxIn, urlIn.toUtf8().constData(), NULL, NULL)) < 0) {
        debug(ret, "打开输入");
        return false;
    }

    if ((ret = avformat_find_stream_info(formatCtxIn, NULL)) < 0) {
        debug(ret, "无流信息");
        return false;
    }

    audioIndex = av_find_best_stream(formatCtxIn, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    videoIndex = av_find_best_stream(formatCtxIn, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (videoIndex < 0) {
        debug(ret, "无视频流");
        return false;
    }

    debug(0, QString("打开成功: %1").arg(urlIn));
    return true;
}

bool FFmpegSaveSimple::openOutput()
{
    int ret = -1;
    saveFile = (!urlOut.contains("://"));
    const char *format = FFmpegSaveHelper::getFormat(urlOut);
    QByteArray urlData = urlOut.toUtf8();
    const char *url = urlData.constData();
    if ((ret = avformat_alloc_output_context2(&formatCtxOut, NULL, format, url)) < 0) {
        debug(ret, "创建输出");
        return false;
    }

    //根据输入流创建输出流
    for (int i = 0; i < formatCtxIn->nb_streams; i++) {
        AVStream *streamIn = formatCtxIn->streams[i];
        AVStream *streamOut = avformat_new_stream(formatCtxOut, NULL);
        if (!streamOut) {
            return false;
        }

        if ((ret = FFmpegHelper::copyContext(streamIn, streamOut)) < 0) {
            debug(ret, "复制参数");
            return false;
        }
    }

    if ((ret = avio_open(&formatCtxOut->pb, url, AVIO_FLAG_WRITE)) < 0) {
        debug(ret, "打开输出");
        return false;
    }

    if ((ret = avformat_write_header(formatCtxOut, NULL)) < 0) {
        debug(ret, "写文件头");
        return false;
    }

    debug(0, QString("开始%2: %1").arg(urlOut).arg(saveFile ? "保存" : "推流"));
    return true;
}

void FFmpegSaveSimple::close()
{
    stopped = false;

    if (formatCtxOut) {
        avformat_close_input(&formatCtxOut);
        formatCtxOut = NULL;
        debug(0, QString("关闭输出: %1").arg(urlOut));
    }

    if (formatCtxIn) {
        avformat_close_input(&formatCtxIn);
        formatCtxIn = NULL;
        debug(0, QString("关闭输入: %1").arg(urlIn));
    }
}

void FFmpegSaveSimple::debug(int ret, const QString &msg)
{
    QString text = (ret < 0 ? QString("%1 错误: %2").arg(msg).arg(FFmpegHelper::getError(ret)) : msg);
    qDebug() << TIMEMS << text;
}

void FFmpegSaveSimple::setUrl(const QString &urlIn, const QString &urlOut)
{
    this->urlIn = urlIn;
    this->urlOut = urlOut;
}

void FFmpegSaveSimple::stop()
{
    this->stopped = true;
    this->wait();
}
