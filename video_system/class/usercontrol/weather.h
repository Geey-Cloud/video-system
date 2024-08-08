#ifndef WEATHER_H
#define WEATHER_H

/**
 * 城市天气类 作者:feiyangqingyun(QQ:517216493) 2021-09-25
 * 1. 指定任意城市获取天气信息。
 * 2. 天气信息包括日期、低温、高温、天气、风力、风向。
 * 3. 查询结果立即返回，包括接收到的数据。
 * 4. 支持获取对应天气的图标值。
 * 5. 自带演示demo省市县下拉联动选择。
 * 6. 支持发送请求获取当前城市定位。
 * 7. 支持指定标签直接显示当前天气。
 * 8. 支持实时采集指定城市天气显示到标签。
 * 9. 天气显示样式支持多种类型风格。
 * 10. 可直接获取天气图形字体。
 */

#include <QObject>
#include <QFont>
#include <QStringList>
#include <QDebug>

class QNetworkAccessManager;
class QNetworkReply;

struct WeatherInfo {
    QString weatherDate;    //天气日期
    QString weatherType;    //天气类型
    QString tempMin;        //最低温度
    QString tempMax;        //最高温度
    QString windValue;      //风力大小
    QString windDirect;     //风向类型

    //重载打印输出格式
    friend QDebug operator << (QDebug debug, const WeatherInfo &weatherInfo) {
        QStringList list;
        list << QString("日期: %1").arg(weatherInfo.weatherDate);
        list << QString("天气: %1").arg(weatherInfo.weatherType);
        list << QString("低温: %1").arg(weatherInfo.tempMin);
        list << QString("高温: %1").arg(weatherInfo.tempMax);
        list << QString("风力: %1").arg(weatherInfo.windValue);
        list << QString("风向: %1").arg(weatherInfo.windDirect);

#if (QT_VERSION >= QT_VERSION_CHECK(5,4,0))
        debug.noquote() << list.join("\n");
#else
        debug << list.join("\n");
#endif
        return debug;
    }
};

#ifdef quc
class Q_DECL_EXPORT WeatherHelper
#else
class WeatherHelper
#endif

{
public:
    //获取天气图形字体
    static QFont getWeatherFont();
    //获取不同天气类型的图标
    static int getWeatherIcon(const QString &type);
    //获取天气内容
    static QString getWeatherText(quint8 style, const QString &city, const QString &temp, const QList<WeatherInfo> &weatherInfos);

    //去掉字符
    static void replace(QString &data);
    //获取请求网页内容
    static QString getHtml(const QString &url);

    //联网自动定位当前城市
    static QString getLocation(QString &ip, QString &province, QString &city, QString &county);

    //传入省份名称获取省份编码
    static QMap<QString, QString> listProvince;
    static QString getProvinceId(const QString &provinceName);

    //传入城市名称获取城市编码
    static QMap<QString, QString> listCity;
    static QString getCityId(const QString &city);
    static QString getCityId(const QString &provinceName, const QString &cityName);

    //传入城市编码获取天气
    static QString getWeather(const QString &name, QString &city, QString &temp, QString &tip, QList<WeatherInfo> &weatherInfos);
};

#ifdef quc
class Q_DECL_EXPORT Weather : public QObject
#else
class Weather : public QObject
#endif

{
    Q_OBJECT
public:
    explicit Weather(QObject *parent = 0);

private:
    //需要采集的城市
    QString city;
    //天气采集定时器
    QTimer *timer;    

public:
    //设置城市名称
    void setCity(const QString &city);

public Q_SLOTS:
    //指定城市查询天气/格式 省份|城市
    void query(const QString &city);
    void query();

    //启动和停止定时器轮询天气
    void start(int interval);
    void stop();

Q_SIGNALS:
    //返回请求数据
    void receiveData(const QString &data);
    //返回天气信息集合
    void receiveWeather(const QString &city, const QString &temp, const QString &tip, const QList<WeatherInfo> &weatherInfos);
};

#endif // WEATHER_H
