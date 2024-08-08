#ifndef FRMWEATHER_H
#define FRMWEATHER_H

#include <QWidget>
#include "weather.h"
class QLabel;
class QComboBox;

namespace Ui {
class frmWeather;
}

class frmWeather : public QWidget
{
    Q_OBJECT

public:
    explicit frmWeather(QWidget *parent = 0);
    ~frmWeather();

protected:
    void showEvent(QShowEvent *);

private:
    Ui::frmWeather *ui;

    //外部天气显示标签
    QLabel *label;
    //天气图形字体
    QFont iconFont;
    //天气采集类
    Weather *weather;    

private slots:
    void initForm();
    void initTable();
    void initConfig();
    void saveConfig();

private slots:
    //返回请求数据
    void receiveData(const QString &data);
    //返回天气信息集合
    void receiveWeather(const QString &city, const QString &temp, const QString &tip, const QList<WeatherInfo> &weatherInfos);
    //设置当前名称
    void setText(QComboBox *cbox, const QString &text);

private slots:
    void on_btnQuery_clicked();
    void on_btnLocation_clicked();
    void on_cboxProvince_currentIndexChanged(int);
    void on_cboxCity_currentIndexChanged(int);
    void on_cboxCounty_currentIndexChanged(int);    

public slots:
    void setLabel(QLabel *label);
    void setConfig(const QString &province, const QString &city, const QString &county, int interval, int style, bool start);

signals:
    void configChanged(const QString &province, const QString &city, const QString &county, int interval, int style);
};

#endif // FRMWEATHER_H
