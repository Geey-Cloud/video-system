#ifndef FRMCONFIGENABLE_H
#define FRMCONFIGENABLE_H

#include <QWidget>

namespace Ui {
class frmConfigEnable;
}

class frmConfigEnable : public QWidget
{
    Q_OBJECT

public:
    explicit frmConfigEnable(QWidget *parent = 0);
    ~frmConfigEnable();

private:
    Ui::frmConfigEnable *ui;

private slots:
    //加载和保存启用配置参数1
    void initEnableConfig1();
    void saveEnableConfig1();

    //加载和保存启用配置参数2
    void initEnableConfig2();
    void saveEnableConfig2();

    //加载和保存启用配置参数3
    void initEnableConfig3();
    void saveEnableConfig3();
};

#endif // FRMCONFIGENABLE_H
