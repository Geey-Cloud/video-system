﻿#ifndef FRMCONFIGSYSTEM_H
#define FRMCONFIGSYSTEM_H

#include <QWidget>

namespace Ui {
class frmConfigSystem;
}

class frmConfigSystem : public QWidget
{
    Q_OBJECT

public:
    explicit frmConfigSystem(QWidget *parent = 0);
    ~frmConfigSystem();

private:
    Ui::frmConfigSystem *ui;    

private slots:
    //初始化窗体数据
    void initForm();

    //加载和保存基础配置参数1
    void initAppConfig1();
    void saveAppConfig1();

    //加载和保存基础配置参数2
    void initAppConfig2();
    void saveAppConfig2();

    //加载和保存基础配置参数3
    void initAppConfig3();
    void saveAppConfig3();    

    //加载和保存颜色配置参数
    void initColorConfig();
    void saveColorConfig();    
};

#endif // FRMCONFIGSYSTEM_H
