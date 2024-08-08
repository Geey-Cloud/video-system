#ifndef FRMVIDEOWIDGETVOLUME_H
#define FRMVIDEOWIDGETVOLUME_H

#include <QWidget>
class VideoWidget;
class BarVolume;
class QSpacerItem;
class QHBoxLayout;

namespace Ui {
class frmVideoWidgetVolume;
}

class frmVideoWidgetVolume : public QWidget
{
    Q_OBJECT

public:
    explicit frmVideoWidgetVolume(int position, VideoWidget *videoWidget, QWidget *parent = 0);
    ~frmVideoWidgetVolume();

protected:
    //尺寸发生变化
    void resizeEvent(QResizeEvent *);

private:
    Ui::frmVideoWidgetVolume *ui;

    //音柱是否垂直
    bool isVertical;
    //音柱摆放位置
    int position;

    //视频控件
    VideoWidget *videoWidget;

    //左右音道
    BarVolume *leftVolume;
    BarVolume *rightVolume;

    //视频控件布局
    QHBoxLayout *hlayout;
    //弹簧控件用于插入布局调整音柱宽度
    QSpacerItem *spaceLeft;
    QSpacerItem *spaceCenter;
    QSpacerItem *spaceRight;

private slots:
    //重置状态数据
    void reset();
    //音频数据振幅
    void receiveLevel(qreal leftLevel, qreal rightLevel);
};

#endif // FRMVIDEOWIDGETVOLUME_H
