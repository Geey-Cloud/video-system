#include "frmdevicetree.h"
#include "ui_frmdevicetree.h"
#include "qthelper.h"
#include "devicehelper.h"

frmDeviceTree::frmDeviceTree(QWidget *parent) : QWidget(parent), ui(new Ui::frmDeviceTree)
{
    ui->setupUi(this);
    this->initForm();
    this->initAction();
    this->initSearch();
}

frmDeviceTree::~frmDeviceTree()
{
    delete ui;
}

bool frmDeviceTree::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->treeWidget->viewport()) {
        QMouseEvent *mouseEvent = (QMouseEvent *)event;
        if (event->type() == QEvent::MouseButtonPress) {
            treePressed = true;
            treePos = mouseEvent->pos();

            //选中节点对应选中视频通道
            //这里后期还要考虑当主码流子码流都分别有不同通道的时候如何选择
            QTreeWidgetItem *item = ui->treeWidget->itemAt(mouseEvent->pos());
            if (item) {
                QString url = item->data(0, Qt::UserRole).toString();
                AppEvent::Instance()->slot_selectVideo(url);
            }
        } else if (event->type() == QEvent::MouseMove) {
            if (treePressed) {
                //当拖动距离大于一个推荐抖动距离时,表示同意已经拖动操作了
                int distance = (mouseEvent->pos() - treePos).manhattanLength();
                if (distance >= QApplication::startDragDistance()) {
                    //设置拖动的图标,暂时没有想要更好的办法

                }
            }
        } else if (event->type() == QEvent::MouseButtonRelease) {
            treePressed = false;
        } else if (event->type() == QEvent::MouseButtonDblClick) {
            //如果双击的是父节点则打开该父节点下的所有,要折叠起来用前面的+号即可
            QTreeWidgetItem *item = ui->treeWidget->itemAt(mouseEvent->pos());
            if (item) {
                QString url = item->data(0, Qt::UserRole).toString();
                AppEvent::Instance()->slot_itemDoubleClicked(url, item->text(0), item->parent() == 0);
                return true;
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

void frmDeviceTree::initForm()
{
    treePressed = false;
    ui->treeWidget->viewport()->installEventFilter(this);

    //初始化树状列表加载设备信息
    DeviceHelper::setTreeWidget(ui->treeWidget);
    DeviceHelper::initDeviceTree();
    DeviceHelper::initVideoIcon();

    connect(AppEvent::Instance(), SIGNAL(exitAll()), this, SLOT(exitAll()));
    connect(AppEvent::Instance(), SIGNAL(fileDragOpen()), this, SLOT(fileDragOpen()));
    connect(AppEvent::Instance(), SIGNAL(selectVideo(QString)), this, SLOT(selectVideo(QString)));
}

void frmDeviceTree::initAction()
{
    //增加树状列表右键菜单
    ui->treeWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
    QStringList listTexts;
    listTexts << "开始对讲" << "输出控制" << "刷新通道" << "刷新所有";
    foreach (QString text, listTexts) {
        QAction *action = new QAction(text, this);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(doAction()));
        ui->treeWidget->addAction(action);
    }
}

void frmDeviceTree::doAction()
{
    //打印具体执行了哪个菜单,可以自行增加处理
    QAction *action = (QAction *)sender();
    QString text = action->text();
    DeviceHelper::addMsg(QString("执行功能 %1").arg(text), 255);
    if (text == "刷新所有") {
        AppEvent::Instance()->slot_restartVideo();
    }
}

void frmDeviceTree::initSearch()
{
    //启用清空按钮
#if (QT_VERSION >= QT_VERSION_CHECK(5,2,0))
    ui->txtSearch->setClearButtonEnabled(true);
#endif

    //设置搜索按钮图标
    IconHelper::setIcon(ui->btnSearch, 0xf002, GlobalConfig::FontSize + 3);

    //识别清空按钮触发事件/指定信号加入队列执行/不然可能还在执行该动作/文本框中还有内容
    QList<QAction *> actions = ui->txtSearch->findChildren<QAction *>();
    foreach (QAction *action, actions) {
        connect(action, SIGNAL(triggered(bool)), this, SLOT(doSearch()), Qt::QueuedConnection);
    }

    //回车或者清空立即处理/也可以改成内容变化就触发
    //connect(ui->txtSearch, SIGNAL(textChanged(QString)), this, SLOT(doSearch()));
    connect(ui->txtSearch, SIGNAL(returnPressed()), this, SLOT(doSearch()));
    connect(ui->btnSearch, SIGNAL(clicked(bool)), this, SLOT(doSearch()));
}

void frmDeviceTree::doSearch()
{
    //根据输入的关键字遍历子节点/符合显示不符合隐藏
    QString key = ui->txtSearch->text().trimmed();
    int count = ui->treeWidget->topLevelItemCount();
    for (int i = 0; i < count; ++i) {
        QTreeWidgetItem *itemNvr = ui->treeWidget->topLevelItem(i);
        //先判断父节点是否包含关键字
        bool exist = itemNvr->text(0).contains(key);

        //逐个遍历子节点
        int size = itemNvr->childCount();
        for (int j = 0; j < size; ++j) {
            QTreeWidgetItem *itemIpc = itemNvr->child(j);
            //采用模糊查找/可以根据自己的需要改成自己的查找方式
            bool contains = itemIpc->text(0).contains(key);
            itemIpc->setHidden(!contains);
            if (contains) {
                exist = true;
            }
        }

        //父节点下的所有子节点没有一个符合要求的则父节点隐藏
        itemNvr->setHidden(!exist);
    }
}

void frmDeviceTree::exitAll()
{
    //记录最后的展开节点索引存储到配置文件
    QStringList listExpandItem;
    int size = ui->treeWidget->topLevelItemCount();
    for (int i = 0; i < size; ++i) {
        if (ui->treeWidget->topLevelItem(i)->isExpanded()) {
            listExpandItem << QString::number(i);
        }
    }

    AppConfig::ExpandItem = listExpandItem.count() > 0 ? listExpandItem.join(",") : "-1";
    AppConfig::writeConfig();
}

void frmDeviceTree::fileDragOpen()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
    //当拖曳过去以后悬停qss效果没有更新
    //模拟鼠标移动来消除这个bug,先移到最后一次的节点的位置,再移到外面
    //经过大量测试发现/当树状节点选中时从本地文件拖曳播放也会触发树状节点拖曳事件/所以需要先屏蔽
    ui->treeWidget->setDragEnabled(false);
    QMouseEvent event0(QEvent::MouseMove, treePos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(ui->treeWidget->viewport(), &event0);
    QMouseEvent event1(QEvent::MouseMove, QPointF(100, 1200), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(ui->treeWidget->viewport(), &event1);
    QMouseEvent event2(QEvent::MouseButtonRelease, QPointF(100, 1200), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(ui->treeWidget->viewport(), &event2);
    ui->treeWidget->setDragEnabled(true);
#endif
}

void frmDeviceTree::selectVideo(const QString &url)
{
    //遍历所有节点是当前url的选中
    QTreeWidgetItemIterator it(ui->treeWidget);
    while (*it) {
        QTreeWidgetItem *item = (*it);
        //最末端节点选中
        if (item->childCount() == 0) {
            QString text = item->data(0, Qt::UserRole).toString();
            bool selected = (text == url);
            item->setSelected(selected);
            //选中对应节点的父节点(不需要这个机制就注释下面几行就行)
            if (selected && AppConfig::TreeRtsp) {
                item->setSelected(false);
                item->parent()->setSelected(true);
            }
        } else {
            item->setSelected(false);
        }
        ++it;
    }
}
