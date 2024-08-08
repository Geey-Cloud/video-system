#ifndef ECHARTS_H
#define ECHARTS_H

#include <QObject>
#include <QStringList>
#include <QColor>

class Echarts : public QObject
{
    Q_OBJECT
public:
    static Echarts *Instance();
    explicit Echarts(QObject *parent = 0);

private:
    static QScopedPointer<Echarts> self;

    int height;             //高度
    bool saveFile;          //保存到文件
    QString title;          //网页标题
    QString fileName;       //文件名称

    QString chartTitle;     //图表标题
    QString tipTitle;       //提示信息标题

    QColor bgColor;         //背景颜色
    QColor textColor;       //文字颜色
    QColor lineColor;       //线条颜色

    QColor tipTextColor;    //提示信息文字颜色
    QColor tipBgColor;      //提示信息背景颜色

    QColor areaColor;       //区域颜色
    QColor areaBgColor;     //区域背景颜色
    QColor areaHoverColor;  //区域悬停颜色
    QColor areaBorderColor; //区域边框颜色

    QStringList cityName;   //城市名称集合
    QStringList cityValue;  //城市对应值集合
    QStringList cityPoint;  //城市经纬度集合
    QStringList cityColor;  //城市颜色集合
    QStringList cityTip;    //自定义提示信息集合

    double zoom;            //放大倍数
    bool roam;              //是否允许鼠标滚轮缩放
    QString mapJsName;      //地图js文件名称
    QString mapAreaName;    //地图区域名称

private:
    //获取值字符串
    QString getValues();
    //获取坐标点字符串
    QString getPoints();

    //获取颜色函数
    QStringList getOptionColor();
    //基础信息字符串
    QStringList getOptionBase();
    //地图参数信息字符串
    QStringList getOptionGeo();

public slots:
    //重置初始值
    void reset();
    //校对数据队列数量
    bool checkCount();

    //填充body生成新的html文件
    QString newChart(const QString &body, const QString &funName = QString(), const QString &funInit = QString());
    //生成闪烁点图
    QString newChartPoint();
    //生成迁徙图
    QString newChartMove(const QString &centerCity);
    //生成仪表盘
    QString newChartGauge(const QString &title, int value);

    //设置画布高度+保存到文件+标题+文件名
    void setHeight(int height);
    void setSaveFile(bool saveFile);
    bool getSaveFile()const;
    void setTitle(const QString &title);
    void setFileName(const QString &fileName);

    //设置标题
    void setChartTitle(const QString &chartTitle);
    void setTipTitle(const QString &tipTitle);

    //设置背景颜色+文字颜色+线条颜色
    void setBgColor(const QColor &bgColor);
    void setTextColor(const QColor &textColor);
    void setLineColor(const QColor &lineColor);

    //设置提示信息文字颜色+背景颜色
    void setTipTextColor(const QColor &tipTextColor);
    void setTipBgColor(const QColor &tipBgColor);

    //设置闪烁点图颜色
    void setAreaColor(const QColor &areaColor);
    void setAreaBgColor(const QColor &areaBgColor);
    void setAreaHoverColor(const QColor &areaHoverColor);
    void setAreaBorderColor(const QColor &areaBorderColor);

    //设置城市名称+值+经纬度+颜色+悬停提示文字
    void setCityName(const QStringList &cityName);
    void setCityValue(const QStringList &cityValue);
    void setCityPoint(const QStringList &cityPoint);
    void setCityColor(const QStringList &cityColor);
    void setCityTip(const QStringList &cityTip);

    //设置放大倍数+缩放+地图js文件名称+地图区域名称
    void setZoom(double zoom);
    void setRoam(bool roam);
    void setMapJsName(const QString &mapJsName);
    void setMapAreaName(const QString &mapAreaName);
};

#endif // ECHARTS_H
