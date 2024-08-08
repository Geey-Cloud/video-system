#pragma execution_character_set("utf-8")
#include "echarts.h"
#include "qmutex.h"
#include "qfile.h"
#include "qtextstream.h"
#include "qdebug.h"

QScopedPointer<Echarts> Echarts::self;
Echarts *Echarts::Instance()
{
    if (self.isNull()) {
        static QMutex mutex;
        QMutexLocker locker(&mutex);
        if (self.isNull()) {
            self.reset(new Echarts);
        }
    }

    return self.data();
}

Echarts::Echarts(QObject *parent) : QObject(parent)
{
    //各种精美图表请参见 https://www.makeapie.com/
    this->reset();
}

QString Echarts::getValues()
{
    //添加对应的值,值越大圆越大
    QStringList values;
    int count = cityName.count();
    for (int i = 0; i < count; ++i) {
        values << QString("{name: '%1', value: %2, color: '%3', tip: '%4'}")
               .arg(cityName.at(i)).arg(cityValue.at(i)).arg(cityColor.at(i)).arg(cityTip.at(i));
    }

    return values.join(",");
}

QString Echarts::getPoints()
{
    //添加对应的经纬度
    QStringList points;
    int count = cityName.count();
    for (int i = 0; i < count; ++i) {
        points << QString("'%1':[%2]").arg(cityName.at(i)).arg(cityPoint.at(i));
    }

    return points.join(",");
}

QStringList Echarts::getOptionColor()
{
    //对应点的颜色,不同点可以不同颜色,颜色值为空则取 areaColor
    QStringList optionColor;
    optionColor << QString("var color = function(params) {");
    //先判断名称是否在需要改变颜色的队列中
    optionColor << QString("  var name = params.name;");
    optionColor << QString("  if (itemNames) {");
    optionColor << QString("    var index = itemNames.indexOf(name);");
    optionColor << QString("    if (index >= 0) {");
    optionColor << QString("      return itemColors[index];");
    optionColor << QString("    }");
    optionColor << QString("  }");
    //返回默认数组中的颜色值
    optionColor << QString("  var color = data[params.dataIndex];");
    optionColor << QString("  color = color.color;");
    optionColor << QString("  if (color == null || color == '') {");
    optionColor << QString("    color = '%1'").arg(areaColor.name());
    optionColor << QString("  }");
    optionColor << QString("  return color;");
    optionColor << QString("}");
    return optionColor;
}

QStringList Echarts::getOptionBase()
{
    QStringList optionBase;
    optionBase << QString("    backgroundColor: 'rgba(%1,%2,%3,%4)',").arg(bgColor.red()).arg(bgColor.green()).arg(bgColor.blue()).arg(bgColor.alphaF());
    optionBase << QString("    tooltip: {");
    optionBase << QString("      trigger: 'item',");
    optionBase << QString("      backgroundColor: 'rgba(%1,%2,%3,0.5)',").arg(tipBgColor.red()).arg(tipBgColor.green()).arg(tipBgColor.blue());
    optionBase << QString("      textStyle: {color: '%1'},").arg(tipTextColor.name());
    //悬停提示的格式 搜索 echart tooltip formatter
    optionBase << QString("      formatter: function (params) {");
    optionBase << QString("        var item = data[params.dataIndex];");
    optionBase << QString("        var tip = item.tip;");
    optionBase << QString("        var color = item.color;");
    optionBase << QString("        if (color == null || color == '') {");
    optionBase << QString("          color = '%1'").arg(areaColor.name());
    optionBase << QString("        }");
    //前面小圆点
    optionBase << QString("        var dot1 = '<i style=\"display:inline-block;width:10px;height:10px;border-radius:50%;margin-right:5px;background:'");
    optionBase << QString("        var dot2 = color + ';\"></i>'");
    optionBase << QString("        var dot = dot1 + dot2");
    optionBase << QString("        if (tip == null || tip == '') {");
    optionBase << QString("          tip = dot + item.name + ' : ' + item.value;");
    optionBase << QString("        } else {");
    optionBase << QString("          tip = dot + item.tip;");
    optionBase << QString("        }");
    optionBase << QString("        return tip;");
    optionBase << QString("      }");
    optionBase << QString("    },");
    optionBase << QString("");
    return optionBase;
}

QStringList Echarts::getOptionGeo()
{
    QStringList optionGeo;
    optionGeo << QString("    geo: {");
    optionGeo << QString("      map: '%1',").arg(mapAreaName);
    optionGeo << QString("      zoom: %1,").arg(zoom);
    optionGeo << QString("      label: {");
    optionGeo << QString("        emphasis: {");
    optionGeo << QString("          show: false");
    optionGeo << QString("        }");
    optionGeo << QString("      },");
    //是否允许缩放
    optionGeo << QString("      roam: %1,").arg(roam ? "true" : "false");
    optionGeo << QString("      itemStyle: {");
    optionGeo << QString("        normal: {");
    optionGeo << QString("          areaColor: '%1',").arg(areaBgColor.name());
    optionGeo << QString("          borderColor: '%1'").arg(areaBorderColor.name());
    optionGeo << QString("        },");
    optionGeo << QString("        emphasis: {");
    optionGeo << QString("          areaColor: '%1'").arg(areaHoverColor.name());
    optionGeo << QString("        }");
    optionGeo << QString("      }");
    optionGeo << QString("    },");
    optionGeo << QString("");
    return optionGeo;
}

void Echarts::reset()
{
    height = 200;
    saveFile = true;
    title = "全国设备分布图(QQ: 517216493)";
    fileName = "map_echarts.html";

    chartTitle = "全国设备分布图";
    tipTitle = "设备分布图";

    bgColor = QColor(64, 74, 89);
    textColor = QColor(255, 255, 255);
    lineColor = QColor(0, 255, 0);

    tipTextColor = QColor(255, 255, 255);
    tipBgColor = QColor(255, 255, 255);

    areaColor = QColor(244, 233, 37);
    areaBgColor = QColor(50, 60, 72);
    areaHoverColor = QColor(42, 51, 61);
    areaBorderColor = QColor(40, 45, 53);

    cityName.clear();
    cityValue.clear();
    cityPoint.clear();
    cityColor.clear();

    cityName << "上海" << "北京" << "成都" << "武汉" << "厦门" << "广州";
    cityValue << "250" << "220" << "150" << "180" << "140" << "170";
    cityPoint << "121.48,31.22" << "116.46,39.92" << "104.06,30.67" << "114.31,30.52" << "118.1,24.46" << "113.23,23.16";

    //初始化空值则取默认值
    int count = cityName.count();
    for (int i = 0; i < count; ++i) {
        cityColor << "";
        cityTip << "";
    }

    zoom = 1.0;
    roam = true;
    mapJsName = "china";
    mapAreaName = "china";
}

bool Echarts::checkCount()
{
    //这里还可以改成取最小的数量
    bool ok = true;
    int cityNameCount = cityName.count();
    int cityValueCount = cityValue.count();
    int cityPointCount = cityPoint.count();
    int cityColorCount = cityColor.count();
    int cityTipCount = cityTip.count();
    if (cityNameCount != cityValueCount || cityValueCount != cityPointCount ||
        cityPointCount != cityColorCount || cityColorCount != cityTipCount) {
        ok = false;
    }

    return ok;
}

QString Echarts::newChart(const QString &body, const QString &funName, const QString &funInit)
{
    QStringList list;

    //头部固定数据
    list << QString("<html>");
    list << QString("<head>");
    list << QString("  <meta charset=\"utf-8\">");
    list << QString("  <title>%1</title>").arg(title);
    //引入webchannel.js
#ifdef webengine
    list << QString("<script type=\"text/javascript\" src=\"qwebchannel.js\"></script>");
#endif
    //引入js文件
    list << QString("  <script type=\"text/javascript\" src=\"echarts.min.js\"></script>");
    list << QString("  <script type=\"text/javascript\" src=\"areajs/%1.js\"></script>").arg(mapJsName);
    list << QString("</head>");

    //隐藏滚动条+设置边距
    list << QString("<style type=\"text/css\">");
    list << QString("  html,body{overflow-x:hidden;overflow-y:hidden;}");
    list << QString("  body{font-family:微软雅黑;margin:0px;border:0px;padding:0px;}");
    list << QString("</style>");

    list << QString("<body>");

    //为echart准备一个dom,传入高度,宽度会自适应
    list << QString("<div id=\"chart\" style=\"height:%1px;\"></div>").arg(height);

    //引入js文件
    list << QString("<script type=\"text/javascript\">");
    //通用返回数据JS函数
    list << QString("  function receiveData(type, data) {");
#ifdef webminiblink
    list << QString("    objName_receiveData(type, data);");
#else
    list << QString("    objName.receiveData(type, data);");
#endif
    list << QString("  }");

    //生成QWebChannel通信对象
    //重复传入对象可能会提示 js: Uncaught TypeError: Cannot read property 'receiveDataFromJs' of undefined
#ifndef webkit
#ifdef webengine
    list << QString("  new QWebChannel(qt.webChannelTransport, function(channel){window.objName = channel.objects.objName;})");
#endif
#endif

    //基于准备好的dom,初始化echarts实例
    list << QString("  var echart = echarts.init(document.getElementById('chart'));");

    //使用刚指定的配置项和数据显示图表。
    if (funName.isEmpty()) {
        list << body;
        list << QString("  echart.setOption(option);");
    } else {
        list << QString("  function %1 {").arg(funName);
        list << body;
        list << QString("    echart.setOption(option);");
        list << QString("  }");
        list << QString("  %1;").arg(funInit);
    }

    //动态改变点的颜色,通过全局变量记住需要改变颜色的名称
    //在返回颜色的时候遍历是否存在需要改变颜色,需要则取设置的颜色,否则取之前的数组的颜色
    //最后通过调用setOption重新设置完整的参数会立即应用
    //如果要动态闪烁的话就需要开个定时器执行
    list << QString("  var itemNames = [];");
    list << QString("  var itemColors = [];");
    list << QString("  function setItemColor(name, color) {");
    //先判断是否已经存在
    list << QString("    var index = itemNames.indexOf(name);");
    list << QString("    if (index >= 0) {");
    list << QString("      itemColors[index] = color;");
    list << QString("    } else {");
    list << QString("      itemNames.push(name);");
    list << QString("      itemColors.push(color);");
    list << QString("    }");
    list << QString("    echart.setOption(option);");
    list << QString("  }");

    //复位之前改变了点的颜色,为空则全部清空
    list << QString("  function clearItemColor(name) {");
    list << QString("    if (name == '') {");
    list << QString("      itemNames = [];");
    list << QString("      itemColors = [];");
    list << QString("      echart.setOption(option);");
    list << QString("      return;");
    list << QString("    }");
    //找到指定名称对应的数组的索引
    list << QString("    var index = itemNames.indexOf(name);");
    list << QString("    if (index >= 0) {");
    list << QString("      itemNames.splice(index, 1);");
    list << QString("      itemColors.splice(index, 1);");
    list << QString("      echart.setOption(option);");
    list << QString("    }");
    list << QString("  }");

    //单击发出信号,增加值的过滤
    list << QString("  echart.on('click', function (params) {");
    list << QString("    var name = params.name;");
    list << QString("    var value = params.value;");
    //过滤未定义undefined或者空值null
    list << QString("    if (name && value && name.length > 0 && value.length > 0) {");
    list << QString("      receiveData('params', name + '|' + value);");
    list << QString("      return;");
    list << QString("    }");
    list << QString("    if (name && name.length > 0) {");
    list << QString("      receiveData('name', name);");
    list << QString("    }");
    list << QString("  });");

    //设置窗口大小变动echarts自动拉伸填充,此方法只能横向拉伸
    list << QString("  window.onresize = echart.resize;");
    //下面的方法用来设置画布的宽度高度
    list << QString("  function resize(width, height) {");
    list << QString("    var chart = document.getElementById('chart');");
    list << QString("    chart.style.width = width + \"px\";");
    list << QString("    chart.style.height = height + \"px\";");
    list << QString("    echart.resize();");
    list << QString("  }");
    list << QString("</script>");

    //末尾固定数据
    list << QString("</body>");
    list << QString("</html>");

    //保存文件
    QString content = list.join("\r\n");
    if (saveFile && !fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QFile::WriteOnly | QFile::Truncate)) {
            QTextStream out(&file);
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
            out.setCodec("utf-8");
#endif
            out << content;
        }
    }

    return content;
}

QString Echarts::newChartPoint()
{
    if (!checkCount()) {
        return "";
    }

    QStringList body;
    body << QString("  var data = [%1]").arg(getValues());
    body << QString("  var geoCoordMap = {%1};").arg(getPoints());

    //转换数据
    QStringList datas;
    datas << QString("var convertData = function (data) {");
    datas << QString("  var res = [];");
    datas << QString("  var count = data.length;");
    datas << QString("  for (var i = 0; i < count; ++i) {");
    datas << QString("    var geoCoord = geoCoordMap[data[i].name];");
    datas << QString("    if (geoCoord) {");
    datas << QString("      res.push({");
    datas << QString("        name: data[i].name,");
    datas << QString("        value: geoCoord.concat(data[i].value)");
    datas << QString("      });");
    datas << QString("    }");
    datas << QString("  }");
    datas << QString("  return res;");
    datas << QString("};");

    datas << QString("var convertedData = [");
    datas << QString("  convertData(data.sort(function(a, b) {");
    datas << QString("    return b.value - a.value;");
    datas << QString("  }).slice(0, %1))").arg(cityName.count());
    datas << QString("];");

    datas << getOptionColor();
    body << "  " + datas.join("\r\n  ");
    body << "";

    QStringList optionBase = getOptionBase();
    QStringList optionGeo = getOptionGeo();

    QStringList optionSeries;
    optionSeries << QString("      name: '%1',").arg(tipTitle);
    optionSeries << QString("      type: 'effectScatter',");
    optionSeries << QString("      coordinateSystem: 'geo',");
    optionSeries << QString("      data: convertedData[0],");
    optionSeries << QString("      symbolSize: function (val) {");
    optionSeries << QString("        return val[2] / 10;");
    optionSeries << QString("      },");
    optionSeries << QString("      showEffectOn: 'render',");
    optionSeries << QString("      rippleEffect: {");
    optionSeries << QString("        brushType: 'stroke'");
    optionSeries << QString("      },");
    optionSeries << QString("      hoverAnimation: true,");
    optionSeries << QString("      label: {");
    optionSeries << QString("        normal: {");
    optionSeries << QString("          formatter: '{b}',");
    //文本的位置,可填 left right top bottom
    optionSeries << QString("          position: 'right',");
    optionSeries << QString("          show: true");
    optionSeries << QString("        }");
    optionSeries << QString("      },");
    optionSeries << QString("      itemStyle: {");
    optionSeries << QString("        normal: {");
    optionSeries << QString("          color: color,");
    optionSeries << QString("          shadowBlur: 10,");
    optionSeries << QString("          shadowColor: '#333'");
    optionSeries << QString("        }");
    optionSeries << QString("      },");
    optionSeries << QString("      zlevel: 1");

    QStringList option;
    option << optionBase.join("\r\n");
    option << optionGeo.join("\r\n");
    option << "    series: [{";
    option << optionSeries.join("\r\n");
    option << "      }";
    option << "    ]";

    body << "  option = {";
    body << option.join("\r\n");
    body << "  };";

    return newChart(body.join("\r\n"));
}

QString Echarts::newChartMove(const QString &centerCity)
{
    if (!checkCount()) {
        return "";
    }

    QStringList body;
    body << QString("  var data = [%1]").arg(getValues());
    body << QString("  var geoCoordMap = {%1};").arg(getPoints());

    //找到中心城市的经纬度,默认值北京
    QString toCity = "北京";
    QString toPoint = "116.46,39.92";
    int index = cityName.indexOf(centerCity);
    if (index >= 0) {
        toCity = centerCity;
        toPoint = cityPoint.at(index);
    }

    //转换数据
    QStringList datas;
    datas << QString("var convertData = function(data) {");
    datas << QString("  var res = [];");
    datas << QString("  var count = data.length;");
    datas << QString("  for (var i = 0; i < count; ++i) {");
    datas << QString("    var dataItem = data[i];");
    datas << QString("    var fromCoord = geoCoordMap[dataItem.name];");
    datas << QString("    var toCoord = [%1];").arg(toPoint);
    datas << QString("    if (fromCoord && toCoord) {");
    datas << QString("      res.push([{");
    datas << QString("        coord: fromCoord,");
    datas << QString("          value: dataItem.value");
    datas << QString("        }, {");
    datas << QString("          coord: toCoord,");
    datas << QString("       }]);");
    datas << QString("    }");
    datas << QString("  }");
    datas << QString("  return res;");
    datas << QString("};");

    datas << getOptionColor();
    body << "  " + datas.join("\r\n  ");
    body << "";

    QStringList optionSeries;
    optionSeries << QString("  var series = [];");
    optionSeries << QString("  [['%1', data]].forEach(function(item, i) {").arg(toCity);
    optionSeries << QString("    series.push({");
    optionSeries << QString("      type: 'lines', zlevel: 2,");
    optionSeries << QString("      effect: {show: true, period: 4, trailLength: 0.02, symbol: 'arrow', symbolSize: 5,},");
    optionSeries << QString("      lineStyle: {normal: {width: 1, opacity: 1, curveness: .3, color: '%1'}},").arg(lineColor.name());
    optionSeries << QString("      data: convertData(item[1])}, {");
    optionSeries << QString("      name: '%1',").arg(tipTitle);
    optionSeries << QString("      type: 'effectScatter', coordinateSystem: 'geo', zlevel: 2,");
    optionSeries << QString("      rippleEffect: {period: 4, brushType: 'stroke', scale: 4},");
    optionSeries << QString("      label: {");
    optionSeries << QString("        normal: {");
    optionSeries << QString("           show: true, position: 'right', offset: [5, 0], fontSize: 15,");
    optionSeries << QString("           formatter: function(params) {return params.data.name;}");
    optionSeries << QString("        },");
    optionSeries << QString("        emphasis: {show: true}");
    optionSeries << QString("      },");
    optionSeries << QString("      symbol: 'circle',");
    optionSeries << QString("      symbolSize: function(val) {");
    optionSeries << QString("        return 6 + val[2] * 5;");
    optionSeries << QString("      },");
    optionSeries << QString("      itemStyle: {normal: {show: true, color: color}},");
    optionSeries << QString("        data: item[1].map(function(dataItem) {");
    optionSeries << QString("          return {name: dataItem.name, value: geoCoordMap[dataItem.name].concat([dataItem.value])};");
    optionSeries << QString("        }),");
    optionSeries << QString("      },");

    //被攻击点,可自行调整对应的值改变大小速度等
    optionSeries << QString("        {");
    optionSeries << QString("          type: 'scatter', coordinateSystem: 'geo', zlevel: 2,");
    optionSeries << QString("          rippleEffect: {period: 4, brushType: 'stroke', scale: 4},");
    optionSeries << QString("          label: {");
    optionSeries << QString("            normal: {");
    optionSeries << QString("              show: true, position: 'right',");
    optionSeries << QString("              color: '#0f0', formatter: '{b}',");
    optionSeries << QString("              textStyle: {color: '#0f0'}");
    optionSeries << QString("            },");
    optionSeries << QString("            emphasis: {show: true, color: '#f60'}");
    optionSeries << QString("          },");
    optionSeries << QString("        }");
    optionSeries << QString("      );");
    optionSeries << QString("    }");
    optionSeries << QString("  );");
    body << optionSeries.join("\r\n") << "";

    QStringList optionBase = getOptionBase();
    QStringList optionGeo = getOptionGeo();

    QString option = QString("%1\r\n%2\r\n    series: series").arg(optionBase.join("\r\n")).arg(optionGeo.join("\r\n"));
    body << "  option = {";
    body << option;
    body << "  };";

    return newChart(body.join("\r\n"));
}

QString Echarts::newChartGauge(const QString &title, int value)
{
    QStringList body;
    body << QString("    var option = {");
    body << QString("    backgroundColor: 'rgba(%1,%2,%3,%4)',").arg(bgColor.red()).arg(bgColor.green()).arg(bgColor.blue()).arg(bgColor.alphaF());
    body << QString("    tooltip: {");
    body << QString("      formatter: '{a} <br/>{b} : {c}%'");
    body << QString("    },");
    body << QString("    toolbox: {");
    body << QString("      feature: {");
    body << QString("        restore: {},");
    body << QString("        saveAsImage: {}");
    body << QString("      }");
    body << QString("    },");
    body << QString("    series: [");
    body << QString("      {");
    body << QString("        type: 'gauge',");
    body << QString("        detail: {");
    body << QString("          formatter: '{value}%',");
    body << QString("          textStyle: {color: '%1', fontSize: 30},").arg(textColor.name());
    body << QString("        },");
    body << QString("        title: {");
    body << QString("          textStyle: {color: '%1', fontSize: 30}").arg(textColor.name());
    body << QString("        },");
    body << QString("        data: [{name: '%1', value: value}]").arg(title);
    body << QString("      }");
    body << QString("    ]");
    body << QString("  };");

    QString funName = QString("setGaugeValue(value)");
    QString funInit = QString("setGaugeValue(%1)").arg(value);
    return newChart(body.join("\r\n  "), funName, funInit);
}

void Echarts::setHeight(int height)
{
    this->height = height;
}

void Echarts::setSaveFile(bool saveFile)
{
    this->saveFile = saveFile;
}

bool Echarts::getSaveFile() const
{
    return this->saveFile;
}

void Echarts::setTitle(const QString &title)
{
    this->title = title;
}

void Echarts::setFileName(const QString &fileName)
{
    this->fileName = fileName;
}

void Echarts::setChartTitle(const QString &chartTitle)
{
    this->chartTitle = chartTitle;
}

void Echarts::setTipTitle(const QString &tipTitle)
{
    this->tipTitle = tipTitle;
}

void Echarts::setBgColor(const QColor &bgColor)
{
    this->bgColor = bgColor;
}

void Echarts::setTextColor(const QColor &textColor)
{
    this->textColor = textColor;
}

void Echarts::setLineColor(const QColor &lineColor)
{
    this->lineColor = lineColor;
}

void Echarts::setTipTextColor(const QColor &tipTextColor)
{
    this->tipTextColor = tipTextColor;
}

void Echarts::setTipBgColor(const QColor &tipBgColor)
{
    this->tipBgColor = tipBgColor;
}

void Echarts::setAreaColor(const QColor &areaColor)
{
    this->areaColor = areaColor;
}

void Echarts::setAreaBgColor(const QColor &areaBgColor)
{
    this->areaBgColor = areaBgColor;
}

void Echarts::setAreaHoverColor(const QColor &areaHoverColor)
{
    this->areaHoverColor = areaHoverColor;
}

void Echarts::setAreaBorderColor(const QColor &areaBorderColor)
{
    this->areaBorderColor = areaBorderColor;
}

void Echarts::setCityName(const QStringList &cityName)
{
    this->cityName = cityName;
}

void Echarts::setCityValue(const QStringList &cityValue)
{
    this->cityValue = cityValue;
}

void Echarts::setCityPoint(const QStringList &cityPoint)
{
    this->cityPoint = cityPoint;
}

void Echarts::setCityColor(const QStringList &cityColor)
{
    this->cityColor = cityColor;
}

void Echarts::setCityTip(const QStringList &cityTip)
{
    this->cityTip = cityTip;
}

void Echarts::setZoom(double zoom)
{
    this->zoom = zoom;
}

void Echarts::setRoam(bool roam)
{
    this->roam = roam;
}

void Echarts::setMapJsName(const QString &mapJsName)
{
    this->mapJsName = mapJsName;
}

void Echarts::setMapAreaName(const QString &mapAreaName)
{
    this->mapAreaName = mapAreaName;
}

