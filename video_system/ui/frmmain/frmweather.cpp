#pragma execution_character_set("utf-8")

#include "frmweather.h"
#include "ui_frmweather.h"
#include "cityhelper.h"
#include "qlabel.h"
#include "qdebug.h"

#define jsonFile ":/data/city.json"
frmWeather::frmWeather(QWidget *parent) : QWidget(parent), ui(new Ui::frmWeather)
{
    ui->setupUi(this);
    this->initForm();
    this->initTable();
    this->initConfig();
}

frmWeather::~frmWeather()
{
    delete ui;
}

void frmWeather::showEvent(QShowEvent *)
{
    on_btnQuery_clicked();
}

void frmWeather::initForm()
{
    label = NULL;
    //实例化图形字体
    iconFont = WeatherHelper::getWeatherFont();

    //关联天气类的信号
    weather = new Weather(this);
    connect(weather, SIGNAL(receiveData(QString)), this, SLOT(receiveData(QString)));
    connect(weather, SIGNAL(receiveWeather(QString, QString, QString, QList<WeatherInfo>)), this, SLOT(receiveWeather(QString, QString, QString, QList<WeatherInfo>)));
}

void frmWeather::initTable()
{
    QList<QString> columnNames;
    QList<int> columnWidths;
    columnNames << "日期" << "天气" << "类型" << "低温" << "高温" << "风力" << "风向";
    columnWidths << 110 << 60 << 60 << 60 << 60 << 60 << 80;

    int columnCount = columnNames.count();
    ui->tableWidget->setColumnCount(columnCount);
    ui->tableWidget->setHorizontalHeaderLabels(columnNames);
    for (int i = 0; i < columnCount; ++i) {
        ui->tableWidget->setColumnWidth(i, columnWidths.at(i));
    }

    //其他属性设置
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->horizontalHeader()->setFixedHeight(25);
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(25);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void frmWeather::initConfig()
{
    //添加省份到下拉框,市区和县城会自动联动
    QStringList province = CityHelper::getProvince(jsonFile);
    ui->cboxProvince->addItems(province);

    ui->cboxInterval->addItem("不采集", 0);
    ui->cboxInterval->addItem("1分钟", 1);
    ui->cboxInterval->addItem("3分钟", 3);
    ui->cboxInterval->addItem("10分钟", 10);
    ui->cboxInterval->addItem("30分钟", 30);
    ui->cboxInterval->addItem("60分钟", 60);
    ui->cboxInterval->setCurrentIndex(0);

    QStringList styles;
    styles << "不显示" << "样式1" << "样式2" << "样式3" << "样式4";
    ui->cboxStyle->addItems(styles);
    ui->cboxStyle->setCurrentIndex(4);

    connect(ui->txtCity, SIGNAL(textChanged(QString)), this, SLOT(saveConfig()));
    connect(ui->cboxInterval, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));
    connect(ui->cboxStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));
}

void frmWeather::saveConfig()
{
    if (!isVisible()) {
        return;
    }

    QString province = ui->cboxProvince->currentText();
    QString city = ui->cboxCity->currentText();
    QString county = ui->cboxCounty->currentText();
    int interval = ui->cboxInterval->itemData(ui->cboxInterval->currentIndex()).toInt();
    int style = ui->cboxStyle->currentIndex();
    emit configChanged(province, city, county, interval, style);

    //立即启动采集
    if (sender() == ui->cboxInterval || sender() == ui->cboxStyle) {
        on_btnQuery_clicked();
    }
}

void frmWeather::receiveData(const QString &data)
{
    ui->textEdit->setText(data);
}

void frmWeather::receiveWeather(const QString &city, const QString &temp, const QString &tip, const QList<WeatherInfo> &weatherInfos)
{
    QStringList list;
    list << QString("城市: %1").arg(city);
    list << QString("温度: %1").arg(temp);
    list << QString("提示: %1").arg(tip);
    ui->txtMsg->setText(list.join("\n"));

    //数据转成表格显示
    int count = weatherInfos.count();
    ui->tableWidget->setRowCount(count);
    for (int i = 0; i < count; ++i) {
        WeatherInfo weatherInfo = weatherInfos.at(i);
        ui->tableWidget->setItem(i, 0, new QTableWidgetItem(weatherInfo.weatherDate));
        ui->tableWidget->setItem(i, 1, new QTableWidgetItem(weatherInfo.weatherType));
        ui->tableWidget->setItem(i, 3, new QTableWidgetItem(weatherInfo.tempMin));
        ui->tableWidget->setItem(i, 4, new QTableWidgetItem(weatherInfo.tempMax));
        ui->tableWidget->setItem(i, 5, new QTableWidgetItem(weatherInfo.windValue));
        ui->tableWidget->setItem(i, 6, new QTableWidgetItem(weatherInfo.windDirect));

        //查找对应天气图标
        int icon = WeatherHelper::getWeatherIcon(weatherInfo.weatherType);
        QTableWidgetItem *itemType = new QTableWidgetItem((QChar)icon);
        iconFont.setPixelSize(18);
        itemType->setFont(iconFont);
        ui->tableWidget->setItem(i, 2, itemType);
    }

    if (count > 1) {
        //今天特殊颜色显示
        for (int i = 0; i <= 6; ++i) {
            //ui->tableWidget->item(1, i)->setBackground(QColor("#22A3A9"));
            //ui->tableWidget->item(1, i)->setForeground(QColor("#FFFFFF"));
        }

        //所有居中
        for (int i = 0; i < count; ++i) {
            for (int j = 0; j <= 6; ++j) {
                ui->tableWidget->item(i, j)->setTextAlignment(Qt::AlignCenter);
            }
        }

        //今天的天气大图标
        int icon = WeatherHelper::getWeatherIcon(weatherInfos.at(1).weatherType);
        iconFont.setPixelSize(100);
        ui->labType->setFont(iconFont);
        ui->labType->setText((QChar)icon);
    }

    //处理天气标签
    int style = ui->cboxStyle->currentIndex();
    QString text = WeatherHelper::getWeatherText(style, city, temp, weatherInfos);
    ui->labWeather->setText(text);

    //如果设置了外部的标签也设置下内容
    if (label) {
        label->setText(text);
    }
}

void frmWeather::setText(QComboBox *cbox, const QString &text)
{
    for (int i = 0; i < cbox->count(); ++i) {
        if (cbox->itemText(i).startsWith(text)) {
            cbox->setCurrentIndex(i);
            break;
        }
    }
}

void frmWeather::on_btnQuery_clicked()
{
    ui->txtMsg->clear();
    ui->labType->clear();
    ui->labWeather->clear();
    ui->tableWidget->setRowCount(0);

    QString city = ui->txtCity->text().trimmed();
    int interval = ui->cboxInterval->itemData(ui->cboxInterval->currentIndex()).toInt();
    weather->setCity(city);
    weather->start(interval);
    weather->query();
}

void frmWeather::on_btnLocation_clicked()
{
    QString ip, province, city, county;
    QString result = WeatherHelper::getLocation(ip, province, city, county);
    ui->textEdit->setText(result);

    this->setText(ui->cboxProvince, province);
    this->setText(ui->cboxCity, city);
    this->setText(ui->cboxCounty, county);
    if (!county.isEmpty()) {
        on_btnQuery_clicked();
    }
}

void frmWeather::on_cboxProvince_currentIndexChanged(int)
{
    QString provinceName = ui->cboxProvince->currentText();
    QStringList city = CityHelper::getCity(jsonFile, provinceName);
    ui->cboxCity->clear();
    ui->cboxCity->addItems(city);
}

void frmWeather::on_cboxCity_currentIndexChanged(int)
{
    QString provinceName = ui->cboxProvince->currentText();
    QString cityName = ui->cboxCity->currentText();
    QStringList county = CityHelper::getCounty(jsonFile, provinceName, cityName);

    //非直辖市的省份/县城如果是区结尾/取对应市名字/比如吉安市下的吉州区应该改名吉安
    //测试发现中央气象台官网还是有部分城市的没有选项/只能用模糊查找
    if (!provinceName.endsWith("市")) {
        county.insert(0, cityName);
        foreach (QString name, county) {
            if (name.endsWith("区")) {
                county.removeOne(name);
            }
        }
    }

    ui->cboxCounty->clear();
    ui->cboxCounty->addItems(county);
}

void frmWeather::on_cboxCounty_currentIndexChanged(int)
{
    QString province = ui->cboxProvince->currentText();
    QString city = ui->cboxCity->currentText();
    QString county = ui->cboxCounty->currentText();

    //特殊部分特殊处理
    if (province.startsWith("台湾")) {
        if (city.startsWith("高雄")) {
            county = "高雄";
        } else if (city.startsWith("台中")) {
            county = "台中";
        } else {
            county = "台北";
        }
    }

    //部分市区很小采用对应的县才能查到天气/需要自行增加
    if (county == "赣州市") {
        county = "赣县";
    } else if (county == "吉安市") {
        county = "吉水县";
    }

    city = QString("%1|%2").arg(province).arg(county);
    ui->txtCity->setText(city);
}

void frmWeather::setLabel(QLabel *label)
{
    this->label = label;
}

void frmWeather::setConfig(const QString &province, const QString &city, const QString &county, int interval, int style, bool start)
{
    ui->cboxProvince->setCurrentIndex(ui->cboxProvince->findText(province));
    ui->cboxCity->setCurrentIndex(ui->cboxCity->findText(city));
    int index = ui->cboxCounty->findText(county);
    ui->cboxCounty->setCurrentIndex(index < 0 ? 0 : index);
    on_cboxCounty_currentIndexChanged(index);

    index = ui->cboxInterval->findData(interval);
    ui->cboxInterval->setCurrentIndex(index);
    ui->cboxStyle->setCurrentIndex(style);

    //启动或者停止采集
    if (start) {
        on_btnQuery_clicked();
    } else {
        weather->stop();
        if (label) {
            label->clear();
        }
    }
}
