#pragma execution_character_set("utf-8")

#include "weather.h"
#include "qfontdatabase.h"
#include "qnetworkaccessmanager.h"
#include "qnetworkreply.h"
#include "qtimer.h"
#include "qdatetime.h"
#include "qeventloop.h"
#include "qlocale.h"
#include "qmutex.h"
#include "qdebug.h"

QFont WeatherHelper::getWeatherFont()
{
    static QFont font;
    QString fontName = "pe-icon-set-weather";
    QString fontFile = ":/font/pe-icon-set-weather.ttf";

    //已经找到天气图形字体
    if (font.family() == "pe-icon-set-weather") {
        return font;
    }

    //判断图形字体是否存在,不存在则加入
    QFontDatabase fontDb;
    if (!fontDb.families().contains(fontName)) {
        int fontId = fontDb.addApplicationFont(fontFile);
        QStringList listName = fontDb.applicationFontFamilies(fontId);
        if (listName.count() == 0) {
            qDebug() << QString("load %1 error").arg(fontName);
        }
    }

    //再次判断是否包含字体名称防止加载失败
    if (fontDb.families().contains(fontName)) {
        font = QFont(fontName);
#if (QT_VERSION >= QT_VERSION_CHECK(4,8,0))
        font.setHintingPreference(QFont::PreferNoHinting);
#endif
    }

    return font;
}

int WeatherHelper::getWeatherIcon(const QString &type)
{
    //没有的或者不正确不贴切的自行对照表找到进行修改
    int icon = 0xe999;
    if (type == "晴") {
        icon = 0xe999;
    } else if (type == "多云") {
        icon = 0xe972;
    } else if (type == "阴") {
        icon = 0xe970;
    } else if (type == "阵雨") {
        icon = 0xe97c;
    } else if (type == "雷阵雨") {
        icon = 0xe9b3;
    } else if (type == "雷阵雨伴有冰雹") {
        icon = 0xe935;
    } else if (type == "雨夹雪") {
        icon = 0xe97a;
    } else if (type == "蒙蒙细雨") {
        icon = 0xe915;
    } else if (type == "小雨") {
        icon = 0xe917;
    } else if (type == "中雨") {
        icon = 0xe978;
    } else if (type == "大雨") {
        icon = 0xe94d;
    } else if (type == "暴雨") {
        icon = 0xe94f;
    } else if (type == "大暴雨") {
        icon = 0xe94f;
    } else if (type == "特大暴雨") {
        icon = 0xe94f;
    } else if (type == "阵雪") {
        icon = 0xe984;
    } else if (type == "小雪") {
        icon = 0xe986;
    } else if (type == "中雪") {
        icon = 0xe996;
    } else if (type == "大雪") {
        icon = 0xe997;
    } else if (type == "暴雪") {
        icon = 0xe901;
    } else if (type == "薄雾") {
        icon = 0xe956;
    } else if (type == "雾") {
        icon = 0xe927;
    } else if (type == "大雾") {
        icon = 0xe929;
    } else if (type == "冻雨") {
        icon = 0xe917;
    } else if (type == "沙尘暴") {
        icon = 0xe9c4;
    } else if (type == "小雨-中雨") {
        icon = 0xe94b;
    } else if (type == "中雨-大雨") {
        icon = 0xe94b;
    } else if (type == "大雨-暴雨") {
        icon = 0xe94b;
    } else if (type == "暴雨-大暴雨") {
        icon = 0xe94f;
    } else if (type == "大暴雨-特大暴雨") {
        icon = 0xe94f;
    } else if (type.contains("雨")) {
        icon = 0xe978;
    } else if (type.contains("雪")) {
        icon = 0xe996;
    } else if (type.contains("霾")) {
        icon = 0xe929;
    }

    return icon;
}

QString WeatherHelper::getWeatherText(quint8 style, const QString &city, const QString &temp, const QList<WeatherInfo> &weatherInfos)
{
    if (style == 0 || weatherInfos.count() <= 1) {
        return QString();
    }

    //取出今天的天气情况设置到标签
    QString text;
    WeatherInfo weatherInfo = weatherInfos.at(0);
    if (style == 1) {
        text = QString("温度: %1  |  天气: %2  |  风力: %3  |  风向: %4")
               .arg(temp).arg(weatherInfo.weatherType).arg(weatherInfo.windValue).arg(weatherInfo.windDirect);
    } else if (style == 2) {
        text = QString("温度: %1 | 天气: %2 | 风力: %3 - %4")
               .arg(temp).arg(weatherInfo.weatherType).arg(weatherInfo.windValue).arg(weatherInfo.windDirect);
    } else if (style == 3) {
        text = QString("当前: %1 | 温度: %2 - %3 | 天气: %4  ")
               .arg(temp).arg(weatherInfo.tempMin).arg(weatherInfo.tempMax).arg(weatherInfo.weatherType);
    } else if (style == 4) {
        if (weatherInfos.count() <= 3) {
            return QString();
        }

        //html表格格式
        QStringList list;
        list << "<table border='0.0' cellspacing='0' cellpadding='2' style='vertical-align:middle;text-align:center;'>";
        //占多行图形字体通用样式 左右设置点边距看起来更舒服
        QString style = "rowspan='2' style='font-size:40px;font-family:pe-icon-set-weather;padding:0px 10px 0px 10px;'";

        //获取日期周几,在Qt6中默认英文需要转换 星期二=Tuesday
        //ddd = 周二 Tue  dddd = 星期二 Tuesday
        QLocale local = QLocale::Chinese;
        QDateTime today = QDateTime::currentDateTime();
        WeatherInfo weatherInfo1 = weatherInfos.at(0);
        WeatherInfo weatherInfo2 = weatherInfos.at(1);
        WeatherInfo weatherInfo3 = weatherInfos.at(2);
        int icon1 = WeatherHelper::getWeatherIcon(weatherInfo1.weatherType);
        int icon2 = WeatherHelper::getWeatherIcon(weatherInfo2.weatherType);
        int icon3 = WeatherHelper::getWeatherIcon(weatherInfo3.weatherType);

        list << "<tr>";
        list << QString("<td %1>%2</td>").arg(style).arg((QChar)icon1);
        list << QString("<td>今天（%1）</td>").arg(local.toString(today, "ddd"));
        list << QString("<td %1>%2</td>").arg(style).arg((QChar)icon2);
        list << QString("<td>明天（%1）</td>").arg(local.toString(today.addDays(1), "ddd"));
        list << QString("<td %1>%2</td>").arg(style).arg((QChar)icon3);
        list << QString("<td>后天（%1）</td>").arg(local.toString(today.addDays(2), "ddd"));
        list << "</tr>";

        list << "<tr>";
        list << QString("<td>%1 - %2</td>").arg(weatherInfo1.tempMin).arg(weatherInfo1.tempMax);
        list << QString("<td>%1 - %2</td>").arg(weatherInfo2.tempMin).arg(weatherInfo2.tempMax);
        list << QString("<td>%1 - %2</td>").arg(weatherInfo3.tempMin).arg(weatherInfo3.tempMax);
        list << "</tr>";

        list << "<tr>";
        QString td = QString("<td style='text-align:center;'>%1</td>").arg(city);
        list << td;
        list << QString("<td>%1 %2</td>").arg(weatherInfo1.weatherType).arg(weatherInfo1.windDirect);
        list << td;
        list << QString("<td>%1 %2</td>").arg(weatherInfo2.weatherType).arg(weatherInfo2.windDirect);
        list << td;
        list << QString("<td>%1 %2</td>").arg(weatherInfo3.weatherType).arg(weatherInfo3.windDirect);
        list << "</tr>";

        list << "</table>";
        text = list.join("");
    }

    return text;
}

void WeatherHelper::replace(QString &data)
{
    data.replace("\"", "");
    data.replace("[", "");
    data.replace("]", "");
    data.replace("{", "");
    data.replace("}", "");
    data.replace(":", ",");
}

QString WeatherHelper::getHtml(const QString &url)
{
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(QNetworkRequest(QUrl(url)));

    QEventLoop eventLoop;
    QObject::connect(&manager, SIGNAL(finished(QNetworkReply *)), &eventLoop, SLOT(quit()));
    eventLoop.exec();

    QByteArray data = reply->readAll();
    reply->deleteLater();
    return QString(data);
}

QString WeatherHelper::getLocation(QString &ip, QString &province, QString &city, QString &county)
{
    //"{\"ret\":\"ok\",\"ip\":\"59.55.229.5\",\"data\":[\"中国\",\"江西\",\"吉安\",\"泰和\",\"电信\",\"343700\",\"0796\"]}"
    QString result = WeatherHelper::getHtml("https://api.ip138.com/ip/?datatype=jsonp&token=f549f04662f186520d8a0d411c5ed337");
    QString data = result;
    WeatherHelper::replace(data);
    QStringList list = data.split(",");
    if (list.count() >= 12) {
        ip = list.at(3);
        province = list.at(6);
        city = list.at(7);
        county = list.at(8);
    }

    return result;
}

QMap<QString, QString> WeatherHelper::listProvince = QMap<QString, QString>();
QString WeatherHelper::getProvinceId(const QString &provinceName)
{
    //如果数据为空则主动获取一次
    QString provinceId;
    if (listProvince.count() == 0) {
        QString data = getHtml("http://www.nmc.cn/rest/province");
        WeatherHelper::replace(data);
        if (!data.isEmpty()) {
            QStringList list = data.split(",");
            int count = list.count();
            for (int i = 0; i < count; i = i + 6) {
                listProvince[list.at(i + 1)] = list.at(i + 3);
            }
        }
    }

    //有些省份是缩略名/需要模糊查找
    //provinceId = listProvince.key(provinceName)
    QMapIterator<QString, QString> it(listProvince);
    while (it.hasNext()) {
        it.next();
        if (it.value().startsWith(provinceName)) {
            provinceId = it.key();
            break;
        }
    }

    return provinceId;
}

//这里按照 省份|城市 作为key
QMap<QString, QString> WeatherHelper::listCity = QMap<QString, QString>();
QString WeatherHelper::getCityId(const QString &city)
{
    QString cityId, cityId2;
    QString city2 = city.split("|").first();
    QMapIterator<QString, QString> it(listCity);
    while (it.hasNext()) {
        it.next();
        QString key = it.key();
        QString value = it.value();

        //有些市中心区会省略/直接取该市第一个
        if (cityId2.isEmpty() && key.startsWith(city2)) {
            cityId2 = value;
        }
        if (city.startsWith(key)) {
            cityId = value;
            break;
        }
    }

    return cityId.isEmpty() ? cityId2 : cityId;
}

QString WeatherHelper::getCityId(const QString &provinceName, const QString &cityName)
{
    //先看下缓存中有没有
    QString city = cityName;
    QString key = QString("%1|%2").arg(provinceName).arg(city);
    QString cityId = WeatherHelper::getCityId(key);
    if (!cityId.isEmpty()) {
        return cityId;
    }

    //获取省份编码
    QString provinceId = WeatherHelper::getProvinceId(provinceName);
    if (provinceId.isEmpty()) {
        return cityId;
    }

    //特殊省份名称过滤
    static QStringList names;
    if (names.count() == 0) {
        names << "内蒙" << "广西" << "西藏" << "宁夏" << "新疆" << "香港" << "澳门";
    }

    //获取城市编码
    QString data = getHtml(QString("http://www.nmc.cn/rest/province/%1").arg(provinceId));
    WeatherHelper::replace(data);
    QStringList list = data.split(",");
    int count = list.count();
    for (int i = 0; i < count; i = i + 8) {
        QString province = list.at(i + 3);
        QString name = province.left(2);
        if (names.contains(name)) {
            province = (name == "内蒙" ? "内蒙古" : name);
        }

        QString key = QString("%1|%2").arg(province).arg(list.at(i + 5));
        listCity[key] = list.at(i + 1);
    }

    //模糊查找/这里存的是 江西省|泰和 /传进来的可能是 江西省|泰和县
    //cityId = listCity.value(key);
    cityId = WeatherHelper::getCityId(key);
    return cityId;
}

QString WeatherHelper::getWeather(const QString &name, QString &city, QString &temp, QString &tip, QList<WeatherInfo> &weatherInfos)
{
    weatherInfos.clear();
    QStringList list = name.split("|");
    if (list.count() != 2) {
        return QString();
    }

    city = list.at(1);
    QString cityId = WeatherHelper::getCityId(list.at(0), list.at(1));
    if (cityId.isEmpty()) {
        return QString();
    }

    //当前天气和风力风向等
    bool existToday = true;
    QString weatherType, weatherTemp, windDirect, windValue;

    QString date = QDateTime::currentDateTime().toString("yyyy/MM/dd");
    QString result = getHtml(QString("http://www.nmc.cn/rest/weather?stationid=%1").arg(cityId));
    QString data = result;
    WeatherHelper::replace(data);
    list = data.split(",");

    //官网应答的数据是固定模板/这里就不用json解析了/按照关键字匹配来解析
    int count = list.count();
    for (int i = 0; i < count; i++) {
        QString text = list.at(i);
        if (text == "publish_time") {
            if (weatherType.isEmpty()) {
                weatherType = list.at(i + 19);
                weatherTemp = list.at(i + 5);
                windDirect = list.at(i + 26);
                windValue = list.at(i + 30);
            }
        } else if (text == "issuecontent") {
            tip = list.at(i + 1);
        } else if (text == "detail") {
            //date,2024-04-03,pt,2024-04-03 12,00,day,weather,info,晴,img,0,temperature,21,wind,direct,西南风,power,微风,night,weather,info,多云,img,1,temperature,9,wind,direct,西南风,power,微风,
            //先判断长度是否符合要求/不然索引越界会崩溃/每行31个数据
            //2024-6-11测试发现传回来的数据多了降水 precipitation /需要将之前的31改成33
            //索性改成了查找两个关键字之间的间隔/万能办法/随便后期怎么变都不会有问题
            //一旦确定一次就是固定值/所以这里用静态变量
            static int index1 = list.indexOf("date", 0);
            static int index2 = list.indexOf("date", index1 + 1);
            static int len = index2 - index1;

            int end = i + (len * 7);
            if (count <= end) {
                break;
            }

            //循环取值
            for (int j = i; j < end; j = j + len) {
                //当天的天气可能过了时间/比如下午访问则没有上午的天气
                QString weatherType = list.at(j + 9);
                if (weatherType == "9999") {
                    existToday = false;
                }

                WeatherInfo info;
                info.weatherDate = list.at(j + 2);
                info.weatherType = weatherType;
                info.tempMax = list.at(j + 13);
                info.tempMin = list.at(j + 26);
                info.windDirect = list.at(j + 16);
                info.windValue = list.at(j + 18);
                weatherInfos << info;
            }
        } else if (text == date) {
            //今天的天气/需要上下结合获取
            if (!existToday && weatherInfos.count() > 0) {
                weatherInfos[0].weatherType = weatherType;
                weatherInfos[0].tempMax = list.at(i + 2);
                weatherInfos[0].tempMin = list.at(i + 4);
                weatherInfos[0].windDirect = windDirect;
                weatherInfos[0].windValue = windValue;
            }
        }
    }

    temp = weatherTemp;
    return result;
}


Weather::Weather(QObject *parent) : QObject(parent)
{
    //定时器查询天气
    city = "北京市|东城区";
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(query()));
    timer->setInterval(60 * 1000);
}

void Weather::setCity(const QString &city)
{
    this->city = city;
}

void Weather::query(const QString &name)
{
    //将每天天气信息存入队列
    QString city, temp, tip;
    QList<WeatherInfo> weatherInfos;
    QString data = WeatherHelper::getWeather(name, city, temp, tip, weatherInfos);
    Q_EMIT receiveData(data);

    //没获取到则搞一个空值/或者返回也行
    if (temp.isEmpty()) {
        return;
    }

    //发出信号通知
    Q_EMIT receiveWeather(city, temp, tip, weatherInfos);
}

void Weather::query()
{
    this->query(this->city);
}

void Weather::start(int interval)
{
    this->stop();

    //采集间隔0表示不采集
    if (interval <= 0) {
        return;
    }

    //启动定时器轮询天气
    timer->start(interval * 60 * 1000);
}

void Weather::stop()
{
    if (timer->isActive()) {
        timer->stop();
    }
}
