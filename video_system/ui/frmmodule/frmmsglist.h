#ifndef FRMMSGLIST_H
#define FRMMSGLIST_H

#include <QWidget>
class frmMsgListItem;

namespace Ui {
class frmMsgList;
}

class frmMsgList : public QWidget
{
    Q_OBJECT

public:
    explicit frmMsgList(QWidget *parent = 0);
    ~frmMsgList();

protected:
    void showEvent(QShowEvent *);
    void resizeEvent(QResizeEvent *);
    bool eventFilter(QObject *watched, QEvent *event);

private:
    Ui::frmMsgList *ui;

    //消息数量
    int msgListCount;
    //当前鼠标按下处的对象
    frmMsgListItem *pressItem;

private slots:
    //初始化窗体数据
    void initForm();
    //初始化卡片消息容器
    void initPanel();
    //初始化右键菜单
    void initAction();
    //执行右键菜单
    void doAction();
    //检查行数
    void checkCount();

public slots:
    //添加卡片图文消息
    frmMsgListItem *creatItem(const QString &msg, const QString &result, const QImage &image, const QString &time);
    void addMsg(const QString &msg, const QString &result, const QImage &image, const QString &time);
    void clearMsg();
};

#endif // FRMMSGLIST_H
