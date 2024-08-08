#include "ffmpegsaveaudio.h"
#include "ffmpegsavehelper.h"

//参考文章 https://blog.csdn.net/fuhanghang/article/details/124821236
#if 0
FFmpegSaveAudio *f = new FFmpegSaveAudio(this);
f->setUrl("f:/mp3/1.mp3", "f:/1.aac");
f->setUrl("f:/mp3/1.aac", "f:/1.mp3");
f->start();
#endif

FFmpegSaveAudio::FFmpegSaveAudio(QObject *parent) : QThread(parent)
{
    stopped = false;
    saveFile = false;

    formatCtxIn = NULL;
    formatCtxOut = NULL;

    streamIn = NULL;
    streamOut = NULL;

    codecCtxIn = NULL;
    codecCtxOut = NULL;

    packetIn = NULL;
    packetOut = NULL;

    frameIn = NULL;
    frameOut = NULL;

    audioFifo = NULL;

    //初始化ffmpeg的库
    FFmpegHelper::initLib();
}

FFmpegSaveAudio::~FFmpegSaveAudio()
{
    this->stop();
    this->close();
}

void FFmpegSaveAudio::run()
{
    duration = 0;
    if (!this->openInput() || !this->openOutput()) {
        this->close();
        return;
    }

    int ret = -1;
    while (!stopped) {
        if ((ret = av_read_frame(formatCtxIn, packetIn)) < 0) {
            if (ret == AVERROR_EOF || ret == AVERROR_EXIT) {
                debug(0, "文件结束");
                break;
            } else {
                debug(ret, "读取出错");
                continue;
            }
        }

        //只处理音频数据
        if (packetIn->stream_index == audioIndex) {
            decode(packetIn);
        }

        //释放对象
        av_packet_unref(packetIn);
    }

    //写文件尾
    if ((ret = av_write_trailer(formatCtxOut)) < 0) {
        debug(ret, "写文件尾");
    }

    this->close();
}

bool FFmpegSaveAudio::openInput()
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
    if (audioIndex < 0) {
        debug(ret, "无音频流");
        return false;
    }

    //创建解码器上下文
    streamIn = formatCtxIn->streams[audioIndex];
    AVCodecx *codecIn = avcodec_find_decoder(FFmpegHelper::getCodecId(streamIn));
    codecCtxIn = avcodec_alloc_context3(codecIn);
    FFmpegHelper::copyContext(codecCtxIn, streamIn, false);

    //打开解码器
    if ((ret = avcodec_open2(codecCtxIn, codecIn, NULL)) < 0) {
        debug(ret, "打开解码");
        return false;
    }

    //创建输入数据包数据帧
    packetIn = FFmpegHelper::creatPacket(NULL);
    frameIn = av_frame_alloc();

    debug(0, QString("打开成功: %1").arg(urlIn));
    return true;
}

bool FFmpegSaveAudio::openOutput()
{
    //根据拓展名指定编码格式
    QString suffix = urlOut.split(".").last();
    suffix = suffix.toLower();
    AVCodecID codecId = AV_CODEC_ID_AAC;
    if (suffix == "mp3") {
        codecId = AV_CODEC_ID_MP3;
    }

    //pcm只能保存到wav
    if (FFmpegHelper::getSampleFormatString(codecCtxIn->sample_fmt) != "fltp") {
        urlOut.replace(suffix, "wav");
        codecId = AV_CODEC_ID_PCM_ALAW;
    }

    int ret = -1;
    saveFile = (!urlOut.contains("://"));
    const char *format = FFmpegSaveHelper::getFormat(urlOut);
    QByteArray urlData = urlOut.toUtf8();
    const char *url = urlData.constData();
    if ((ret = avformat_alloc_output_context2(&formatCtxOut, NULL, format, url)) < 0) {
        debug(ret, "创建输出");
        return false;
    }

    //创建编码器上下文/参数与输入保持一致
    AVCodecx *codecOut = avcodec_find_encoder(codecId);
    codecCtxOut = avcodec_alloc_context3(codecOut);
    codecCtxOut->codec_type = AVMEDIA_TYPE_AUDIO;
    int channels = FFmpegHelper::getChannel(codecCtxIn);
    FFmpegHelper::initChannel(codecCtxOut, channels);
    codecCtxOut->sample_rate = codecCtxIn->sample_rate;
    codecCtxOut->sample_fmt = codecCtxIn->sample_fmt;
    codecCtxOut->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    //打开编码器
    if ((ret = avcodec_open2(codecCtxOut, codecOut, NULL)) < 0) {
        debug(ret, "打开编码");
        return false;
    }

    //创建输出音频流并复制各种参数
    streamOut = avformat_new_stream(formatCtxOut, codecOut);
    FFmpegHelper::copyContext(codecCtxOut, streamOut, true);

    if ((ret = avio_open(&formatCtxOut->pb, url, AVIO_FLAG_WRITE)) < 0) {
        debug(ret, "打开输出");
        return false;
    }

    if ((ret = avformat_write_header(formatCtxOut, NULL)) < 0) {
        debug(ret, "写文件头");
        return false;
    }

    //创建输出数据包数据帧
    packetOut = FFmpegHelper::creatPacket(NULL);
    frameOut = av_frame_alloc();
    FFmpegHelper::initFrame(frameOut, codecCtxOut);
    av_frame_get_buffer(frameOut, 0);

    //创建音频缓冲
    audioFifo = av_audio_fifo_alloc(codecCtxOut->sample_fmt, channels, codecCtxOut->frame_size);

    debug(0, QString("开始%2: %1").arg(urlOut).arg(saveFile ? "保存" : "推流"));
    return true;
}

void FFmpegSaveAudio::close()
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

    if (codecCtxIn) {
        avcodec_free_context(&codecCtxIn);
        codecCtxIn = NULL;
    }

    if (codecCtxOut) {
        avcodec_free_context(&codecCtxOut);
        codecCtxOut = NULL;
    }

    FFmpegHelper::freePacket(packetIn);
    FFmpegHelper::freePacket(packetOut);
    FFmpegHelper::freeFrame(frameIn);
    FFmpegHelper::freeFrame(frameOut);

    packetIn = NULL;
    packetOut = NULL;
    frameIn = NULL;
    frameOut = NULL;

    if (audioFifo) {
        av_audio_fifo_free(audioFifo);
        audioFifo = NULL;
    }
}

void FFmpegSaveAudio::decode(AVPacket *packet)
{
    int ret = -1;
    int size = 0;

#if (FFMPEG_VERSION_MAJOR < 3)
    avcodec_decode_audio4(codecCtxIn, frameIn, &ret, packet);
    if (ret < 0) {
        debug(ret, "解码出错");
        return;
    } else {
        goto end;
    }
#else
    if ((ret = avcodec_send_packet(codecCtxIn, packet)) < 0) {
        debug(ret, "解码出错");
        return;
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(codecCtxIn, frameIn);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            debug(ret, "解码失败");
            return;
        } else {
            goto end;
        }
    }
#endif

    return;
end:
    //采样点一样或者编码到wav则直接编码保存/否则加入缓存队列
    if (frameIn->nb_samples == frameOut->nb_samples || codecCtxOut->codec_id == AV_CODEC_ID_PCM_ALAW) {
        this->encode(frameIn);
    } else {
        size = av_audio_fifo_size(audioFifo);
        av_audio_fifo_realloc(audioFifo, size + frameIn->nb_samples);
        av_audio_fifo_write(audioFifo, (void **)frameIn->data, frameIn->nb_samples);

        //数据到了要求的大小则取出来进行编码
        size = frameOut->nb_samples;
        while (av_audio_fifo_size(audioFifo) > size) {
            int ret = av_audio_fifo_read(audioFifo, (void **)frameOut->data, size);
            if (ret < 0) {
                debug(ret, "取出出错");
                return;
            }

            this->encode(frameOut);
        }
    }
}

void FFmpegSaveAudio::encode(AVFrame *frame)
{
    int ret = -1;
    duration += frame->nb_samples;
    frame->pts = duration;

#if (FFMPEG_VERSION_MAJOR < 3)
    avcodec_encode_audio2(codecCtxOut, packetOut, frame, &ret);
    if (ret < 0) {
        debug(ret, "编码出错");
        return;
    } else {
        goto end;
    }
#else
    if ((ret = avcodec_send_frame(codecCtxOut, frame)) < 0) {
        debug(ret, "编码出错");
        return;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(codecCtxOut, packetOut);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            debug(ret, "编码失败");
            return;
        } else {
            goto end;
        }
    }
#endif

    return;
end:
    packetOut->stream_index = 0;
    debug(0, "写入一帧");
    if ((ret = av_interleaved_write_frame(formatCtxOut, packetOut)) < 0) {
        debug(ret, "写数据包");
        return;
    }
}

void FFmpegSaveAudio::debug(int ret, const QString &msg)
{
    QString text = (ret < 0 ? QString("%1 错误: %2").arg(msg).arg(FFmpegHelper::getError(ret)) : msg);
    qDebug() << TIMEMS << text;
}

void FFmpegSaveAudio::setUrl(const QString &urlIn, const QString &urlOut)
{
    this->urlIn = urlIn;
    this->urlOut = urlOut;
}

void FFmpegSaveAudio::stop()
{
    this->stopped = true;
    this->wait();
}
