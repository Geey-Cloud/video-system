#include "ffmpegwebsocket.h"

void FFmpegWebSocket::init(FFmpegWebSocket **obj, const QString &url, const QString &flag, MediaType mediaType)
{
    if (mediaType == MediaType_WebSocket) {
        if (!(*obj)) {
            (*obj) = new FFmpegWebSocket;
            (*obj)->setObjectName(QString("FFmpegWebSocket_%1").arg(flag));
        }

        (*obj)->setUrl(url);
        (*obj)->start();
    }
}

void FFmpegWebSocket::free(FFmpegWebSocket **obj)
{
    if ((*obj)) {
        (*obj)->quit();
        (*obj)->wait();
        (*obj)->free();
        (*obj)->deleteLater();
        (*obj) = NULL;
    }
}

FFmpegWebSocket::FFmpegWebSocket(QObject *parent) : QThread(parent)
{
    //可以自行调整下面几个值
    connTimeout = 1500;
    //值越小打开速度越快/分辨率越大值建议越大
    minSizeOpen = 100 * 1024;
    //值越小越容易花屏/控制多大缓存才能读取
    minSizeRead = 1 * 1024;
    //整个缓存队列的大小
    fifoSize = 10 * 1024 * 1024;

    fifo = NULL;
    avioCtx = NULL;
    webSocket = NULL;    
}

FFmpegWebSocket::~FFmpegWebSocket()
{
}

void FFmpegWebSocket::run()
{
    this->initWeb();
    this->closeWeb();
    this->freeFifo();
    this->initFifo();
    this->openWeb();
    this->exec();
}

void FFmpegWebSocket::initWeb()
{
#ifdef websocket
    if (!webSocket) {
        qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState");
        webSocket = new QWebSocket("WebSocket", QWebSocketProtocol::VersionLatest);
        connect(webSocket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(binaryMessageReceived(QByteArray)), Qt::DirectConnection);
    }
#endif
}

void FFmpegWebSocket::freeWeb()
{
#ifdef websocket
    if (webSocket) {
        disconnect(webSocket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(binaryMessageReceived(QByteArray)));
        this->closeWeb();
        webSocket->deleteLater();
        webSocket = NULL;
    }
#endif
}

#ifdef webhelper
#include "webhelper.h"
#endif
void FFmpegWebSocket::openWeb()
{
#ifdef websocket
    QString url = this->url;
#ifdef webhelper
    url = WebHelper::checkUrl(url, connTimeout);
#endif
    if (!url.isEmpty()) {
        webSocket->open(QUrl(url));
    }
#endif
}

void FFmpegWebSocket::closeWeb()
{
#ifdef websocket
    //如果不加判断直接关闭会提示 QNativeSocketEngine::write() was not called in QAbstractSocket::ConnectedState
    if (webSocket && webSocket->state() != QAbstractSocket::ConnectingState) {
        webSocket->close();
    }
#endif
}

void FFmpegWebSocket::initFifo()
{
    if (!fifo) {
#if (FFMPEG_VERSION_MAJOR < 5)
        fifo = av_fifo_alloc(fifoSize);
#else
        fifo = av_fifo_alloc2(fifoSize, sizeof(uint8_t), AV_FIFO_FLAG_AUTO_GROW);
#endif
        uint8_t *avioBuf = (uint8_t *)av_malloc(fifoSize);
        avioCtx = avio_alloc_context(avioBuf, fifoSize, 0, fifo, &read_packet, NULL, NULL);
    }
}

void FFmpegWebSocket::freeFifo()
{
    if (fifo) {
#if (FFMPEG_VERSION_MAJOR < 5)
        av_fifo_reset(fifo);
        av_fifo_freep(&fifo);
#else
        av_fifo_reset2(fifo);
        av_fifo_freep2(&fifo);
#endif

        av_freep(&avioCtx->buffer);
#if (FFMPEG_VERSION_MAJOR < 3)
        av_free(avioCtx);
#else
        avio_context_free(&avioCtx);
#endif
        fifo = NULL;
    }
}

void FFmpegWebSocket::binaryMessageReceived(const QByteArray &message)
{
    int size = message.size();
    const void *data = message.data();
#if (FFMPEG_VERSION_MAJOR < 5)
    int num = av_fifo_space(fifo);
    int ret = av_fifo_generic_write(fifo, (void *)data, size, NULL);
#else
    int num = av_fifo_can_write(fifo);
    int ret = av_fifo_write(fifo, data, size);
#endif
    //qDebug() << TIMEMS << num << ret << size;
}

int FFmpegWebSocket::read_packet(void *opaque, uint8_t *buf, int size)
{
    AVFifox *fifo = (AVFifox *)opaque;
    int ret = -1;
    if (!fifo) {
        return ret;
    }

#if (FFMPEG_VERSION_MAJOR < 5)
    int availableBytes = av_fifo_size(fifo);
#else
    int availableBytes = av_fifo_can_read(fifo);
#endif
    if (availableBytes <= 0) {
        return ret;
    }

    int len = qMin(availableBytes, size);
#if (FFMPEG_VERSION_MAJOR < 5)
    ret = av_fifo_generic_read(fifo, buf, len, NULL);
#else
    ret = av_fifo_read(fifo, buf, len);
#endif
    return len;
}

bool FFmpegWebSocket::canOpen()
{
    return this->checkData(minSizeOpen);
}

bool FFmpegWebSocket::canRead()
{
    return this->checkData(minSizeRead);
}

bool FFmpegWebSocket::checkData(int size)
{
    if (!fifo) {
        return false;
    }

#if (FFMPEG_VERSION_MAJOR < 5)
    return (av_fifo_size(fifo) > size);
#else
    return (av_fifo_can_read(fifo) > size);
#endif
}

AVIOContext *FFmpegWebSocket::getCtx()
{
    return this->avioCtx;
}

void FFmpegWebSocket::setUrl(const QString &url)
{
    this->url = url;
}

void FFmpegWebSocket::reopen()
{
    this->quit();
    this->wait();
    this->start();
}

void FFmpegWebSocket::free()
{
    this->freeWeb();
    this->freeFifo();
}
