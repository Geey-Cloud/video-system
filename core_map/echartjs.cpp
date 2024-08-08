#pragma execution_character_set("utf-8")
#include "echartjs.h"
#include "qfile.h"
#include "qfileinfo.h"
#include "qdir.h"
#include "qtextstream.h"
#include "qdatetime.h"
#include "qdebug.h"

#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
#include "qjsonarray.h"
#include "qjsondocument.h"
#include "qjsonobject.h"
#include "qjsonvalue.h"
#endif

void EchartJs::saveJs(const QString &jsonFile, const QString &jsFile, const QString &cityName)
{
    QStringList list;

    //头部固定数据
    list << "(function (root, factory) {";
    list << "    if (typeof define === 'function' && define.amd) {";
    list << "        // AMD. Register as an anonymous module.";
    list << "        define(['exports', 'echarts'], factory);";
    list << "    } else if (typeof exports === 'object' && typeof exports.nodeName !== 'string') {";
    list << "        // CommonJS";
    list << "        factory(exports, require('echarts'));";
    list << "    } else {";
    list << "        // Browser globals";
    list << "        factory({}, root.echarts);";
    list << "    }";
    list << "} (this, function (exports, echarts) {";
    list << "    var log = function (msg) {";
    list << "        if (typeof console !== 'undefined') {";
    list << "            console && console.error && console.error(msg);";
    list << "        }";
    list << "    }";
    list << "    if (!echarts) {";
    list << "        log('ECharts is not Loaded');";
    list << "        return;";
    list << "    }";
    list << "    if (!echarts.registerMap) {";
    list << "        log('ECharts Map is not loaded');";
    list << "        return;";
    list << "    }";

    //从json文件读取数据
    QString body;
    QFile fileJson(jsonFile);
    if (fileJson.open(QFile::ReadOnly | QFile::Text)) {
        body = fileJson.readAll();
        fileJson.close();
    }

    //加入到数据中
    list << QString("    echarts.registerMap('%1', %2);").arg(cityName).arg(body);
    list << "}));";

    //保存数据到js文件
    QFile fileJs(jsFile);
    //文件夹不存在则先生成文件夹
    QString path = QFileInfo(fileJs).path();
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkdir(path);
    }

    //每次打开都清空
    if (fileJs.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&fileJs);
        out << list.join("\r\n");
    }
}

QStringList EchartJs::getInfoFromJson(const QString &jsonFile)
{
    QByteArray data;
    QFile fileJson(jsonFile);
    if (fileJson.open(QFile::ReadOnly | QFile::Text)) {
        data = fileJson.readAll();
        fileJson.close();
    }

    return getInfoFromData(data);
}

QStringList EchartJs::getInfoFromJs(const QString &jsFile)
{
    QByteArray data;
    QFile fileJs(jsFile);
    if (fileJs.open(QFile::ReadOnly | QFile::Text)) {
        while (!fileJs.atEnd()) {
            QByteArray line = fileJs.readLine();
            line = line.trimmed();
            if (line.startsWith("echarts.registerMap")) {
                int index = line.indexOf("{");
                data = line.mid(index, line.length() - index - 2);
            }
        }

        fileJs.close();
    }

    return getInfoFromData(data);
}

QStringList EchartJs::getInfoFromData(const QByteArray &data)
{
    //取出对应的城市名称和经纬度
    //以下两种方法测试过解析时间,json大概1ms,字符串分割大概5ms,json方法更快
    QStringList result;
#if (QT_VERSION < QT_VERSION_CHECK(5,0,0))
    //采用字符串分割方法解析
    QString temp = data;
    temp = temp.mid(0, temp.length() - 24);
    QString flag = "properties";
    int count = temp.count();
    for (int i = 0; i < count; ++i) {
        QString str = temp.mid(i, 10);
        if (str != flag) {
            continue;
        }

        str = temp.mid(i, 100);
        str = str.mid(13, str.indexOf("},") - 13);
        str.replace("}", "");
        //到这步数据已经变成 "cp":[121.490317,31.222771],"name":"黄浦区","childNum":1
        //cp name的顺序可能不一样,所以需要分割字符串重新判断
        QString name, cp;
        QStringList list = str.split(",");
        foreach (QString s, list) {
            if (s.startsWith("\"cp\"")) {
                cp = s.mid(6, s.length());
            } else if (s.startsWith("\"name\"")) {
                name = s.mid(8, s.length());
                name.replace("\"", "");
            } else if (s.startsWith("\"childNum\"")) {

            } else {
                //经纬度会拆分成两部分,一部分在这里 31.222771]
                cp = QString("%1,%2").arg(cp).arg(s.left(s.length() - 1));
            }
        }

        result << QString("%1|%2").arg(name).arg(cp);
    }
#else
    //采用qt内置的json方法解析
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    if (error.error == QJsonParseError::NoError) {
        QJsonObject rootObj = jsonDoc.object();
        //qDebug() << rootObj.keys();
        if (!rootObj.contains("features")) {
            return QStringList();
        }

        QJsonArray features = rootObj.value("features").toArray();
        int count = features.count();
        for (int i = 0; i < count; ++i) {
            QJsonObject subObj = features.at(i).toObject();
            if (!subObj.contains("properties")) {
                continue;
            }

            QJsonObject nodeObj = subObj.value("properties").toObject();
            QJsonArray array = nodeObj.value("cp").toArray();
            QStringList list;
            for (int j = 0; j < array.count(); ++j) {
                list << QString::number(array.at(j).toDouble());
            }

            QString name = nodeObj.value("name").toString();
            QString cp = list.join(",");
            result << QString("%1|%2").arg(name).arg(cp);
        }
    }
#endif
    return result;
}
