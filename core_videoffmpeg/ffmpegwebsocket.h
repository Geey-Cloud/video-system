#ifndef FFMPEGWEBSOCKET_H
#define FFMPEGWEBSOCKET_H

#include "ffmpeginclude.h"
#include "videostruct.h"
#include "qthread.h"

#ifdef websocket
#include <QWebSocket>
#else
class QWebSocket;
#endif

class FFmpegWebSocket : public QThread
{
    Q_OBJECT
public:
    //初始化和释放
    static void init(FFmpegWebSocket **obj, const QString &url, const QString &flag, MediaType mediaType);
    static void free(FFmpegWebSocket **obj);

    explicit FFmpegWebSocket(QObject *parent = 0);
    ~FFmpegWebSocket();

protected:
    void run();

private:
    //通信地址
    QString url;
    //通信对象
    QWebSocket *webSocket;

    //连接请求超时时间
    int connTimeout;
    //打开最小缓存
    int minSizeOpen;
    //读取最小缓存
    int minSizeRead;
    //分配缓存大小
    int fifoSize;

    //缓存数据
    AVFifox *fifo;
    //操作上下文
    AVIOContext *avioCtx;

private:
    //初始化和释放网络通信对象
    void initWeb();
    void freeWeb();

    //打开和关闭地址
    void openWeb();
    void closeWeb();

    //初始化和释放对象
    void initFifo();
    void freeFifo();

private slots:
    //收到二进制数据
    void binaryMessageReceived(const QByteArray &message);

public:
    //读取数据回调
    static int read_packet(void *opaque, uint8_t *buf, int size);

    //根据缓存数据大小判断是否可以打开和读取
    bool canOpen();
    bool canRead();
    bool checkData(int size);

    //返回对象
    AVIOContext *getCtx();
    //设置地址
    void setUrl(const QString &url);

    //重新打开
    Q_INVOKABLE void reopen();
    //释放资源
    Q_INVOKABLE void free();
};

#endif // FFMPEGWEBSOCKET_H
