#ifndef FRMCONFIGOTHER_H
#define FRMCONFIGOTHER_H

#include <QWidget>

namespace Ui {
class frmConfigOther;
}

class frmConfigOther : public QWidget
{
    Q_OBJECT

public:
    explicit frmConfigOther(QWidget *parent = 0);
    ~frmConfigOther();

private:
    Ui::frmConfigOther *ui;

private slots:
    //初始化界面数据
    void initForm();

    //加载和保存地图配置参数
    void initMapConfig();
    void saveMapConfig();

    //加载和保存路径配置参数
    void initPathConfig();
    void savePathConfig();

    //加载和保存串口配置参数
    void initComConfig();
    void saveComConfig();

    //加载和保存网络配置参数
    void initNetConfig();
    void saveNetConfig();

private slots:
    void selectPath();
    void on_btnComImage_clicked();
    void on_btnInfo_clicked();
};

#endif // FRMCONFIGOTHER_H
