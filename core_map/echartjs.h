#ifndef ECHARTJS_H
#define ECHARTJS_H

#include <QStringList>

class EchartJs
{
public:
    //下载地址 https://github.com/apache/echarts/tree/master/test/data/map/json
    //传入json文件转成js文件
    static void saveJs(const QString &jsonFile, const QString &jsFile, const QString &cityName);
    //获取json文件中的名字和经纬度
    static QStringList getInfoFromJson(const QString &jsonFile);
    //获取js文件中的名字和经纬度
    static QStringList getInfoFromJs(const QString &jsFile);
    //获取数据中的名字和经纬度
    static QStringList getInfoFromData(const QByteArray &data);
};

#endif // ECHARTJS_H
