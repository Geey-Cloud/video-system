#ifndef FRMCONFIGVIDEO_H
#define FRMCONFIGVIDEO_H

#include <QWidget>

namespace Ui {
class frmConfigVideo;
}

class frmConfigVideo : public QWidget
{
    Q_OBJECT

public:
    explicit frmConfigVideo(QWidget *parent = 0);
    ~frmConfigVideo();

private:
    Ui::frmConfigVideo *ui;

private slots:
    void initDebugInfo();

    //加载和保存视频配置参数1
    void initVideoConfig1();
    void saveVideoConfig1();

    //加载和保存视频配置参数2
    void initVideoConfig2();
    void saveVideoConfig2();

    //加载和保存视频配置参数3
    void initVideoConfig3();
    void saveVideoConfig3();

    //加载和保存视频配置参数4
    void initVideoConfig4();
    void saveVideoConfig4();
};

#endif // FRMCONFIGVIDEO_H
