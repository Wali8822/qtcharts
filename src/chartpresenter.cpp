#include "qchart.h"
#include "qchartaxis.h"
#include "chartpresenter_p.h"
#include "chartdataset_p.h"
#include "charttheme_p.h"
//series
#include "barchartseries.h"
#include "stackedbarchartseries.h"
#include "percentbarchartseries.h"
#include "qlinechartseries.h"
#include "qpieseries.h"
//items
#include "axisitem_p.h"
#include "bargroup.h"
#include "stackedbargroup.h"
#include "linechartitem_p.h"
#include "percentbargroup.h"
#include "linechartanimationitem_p.h"
#include "piepresenter.h"

QTCOMMERCIALCHART_BEGIN_NAMESPACE

ChartPresenter::ChartPresenter(QChart* chart,ChartDataSet* dataset):QObject(chart),
m_chart(chart),
m_dataset(dataset),
m_chartTheme(0),
m_domainIndex(0),
m_marginSize(0),
m_rect(QRectF(QPoint(0,0),m_chart->size()))
{
    setChartTheme(QChart::ChartThemeDefault);
    createConnections();
    createDeafultAxis();
}

ChartPresenter::~ChartPresenter()
{
}

void ChartPresenter::createDeafultAxis()
{
    //default axis
    QChartAxis* axisX = new QChartAxis(this);
    QChartAxis* axisY = new QChartAxis(this);

    m_axis << new AxisItem(axisX,AxisItem::X_AXIS,m_chart);
    m_axis << new AxisItem(axisY,AxisItem::Y_AXIS,m_chart);

    foreach(AxisItem* item, m_axis) {
        QObject::connect(this,SIGNAL(geometryChanged(const QRectF&)),item,SLOT(handleGeometryChanged(const QRectF&)));
        QObject::connect(m_dataset,SIGNAL(domainChanged(const Domain&)),item,SLOT(handleDomainChanged(const Domain&)));
    }
}

void ChartPresenter::createConnections()
{
    QObject::connect(m_chart,SIGNAL(geometryChanged()),this,SLOT(handleGeometryChanged()));
    QObject::connect(m_dataset,SIGNAL(seriesAdded(QChartSeries*)),this,SLOT(handleSeriesAdded(QChartSeries*)));
}

void ChartPresenter::handleGeometryChanged()
{
    m_rect = QRectF(QPoint(0,0),m_chart->size());
    m_rect.adjust(m_marginSize,m_marginSize, -m_marginSize, -m_marginSize);
    Q_ASSERT(m_rect.isValid());
    emit geometryChanged(m_rect);
}

int ChartPresenter::margin() const
{
    return m_marginSize;
}

void ChartPresenter::setMargin(int margin)
{
    m_marginSize = margin;
}

void ChartPresenter::handleSeriesAdded(QChartSeries* series)
{
    switch(series->type())
    {
        case QChartSeries::SeriesTypeLine: {
            QLineChartSeries* lineSeries = static_cast<QLineChartSeries*>(series);
            LineChartItem* item = new LineChartAnimationItem(this,lineSeries,m_chart);
            m_chartTheme->decorate(item,lineSeries,m_chartItems.count());
            QObject::connect(this,SIGNAL(geometryChanged(const QRectF&)),item,SLOT(handleGeometryChanged(const QRectF&)));
            QObject::connect(m_dataset,SIGNAL(domainChanged(const Domain&)),item,SLOT(handleDomainChanged(const Domain&)));
            QObject::connect(lineSeries,SIGNAL(changed(int)),item,SLOT(handleModelChanged(int)));
            m_chartItems.insert(series,item);
            break;
        }

        case QChartSeries::SeriesTypeBar: {
            BarChartSeries* barSeries = static_cast<BarChartSeries*>(series);
            BarGroup* item = new BarGroup(*barSeries,m_chart);
            m_chartTheme->decorate(item,barSeries,m_chartItems.count());
            QObject::connect(this,SIGNAL(geometryChanged(const QRectF&)),item,SLOT(handleGeometryChanged(const QRectF&)));
            QObject::connect(m_dataset,SIGNAL(domainChanged(const Domain&)),item,SLOT(handleDomainChanged(const Domain&)));
            QObject::connect(barSeries,SIGNAL(changed(int)),item,SLOT(handleModelChanged(int)));
            m_chartItems.insert(series,item);
            // m_axisXItem->setVisible(false);
            break;
        }

        case QChartSeries::SeriesTypeStackedBar: {

            StackedBarChartSeries* stackedBarSeries = static_cast<StackedBarChartSeries*>(series);
            StackedBarGroup* item = new StackedBarGroup(*stackedBarSeries,m_chart);
            m_chartTheme->decorate(item,stackedBarSeries,m_chartItems.count());
            QObject::connect(this,SIGNAL(geometryChanged(const QRectF&)),item,SLOT(handleGeometryChanged(const QRectF&)));
            QObject::connect(m_dataset,SIGNAL(domainChanged(const Domain&)),item,SLOT(handleDomainChanged(const Domain&)));
            QObject::connect(stackedBarSeries,SIGNAL(changed(int)),item,SLOT(handleModelChanged(int)));
            m_chartItems.insert(series,item);
            break;
        }

        case QChartSeries::SeriesTypePercentBar: {

            PercentBarChartSeries* percentBarSeries = static_cast<PercentBarChartSeries*>(series);
            PercentBarGroup* item = new PercentBarGroup(*percentBarSeries,m_chart);
            m_chartTheme->decorate(item,percentBarSeries ,m_chartItems.count());
            QObject::connect(this,SIGNAL(geometryChanged(const QRectF&)),item,SLOT(handleGeometryChanged(const QRectF&)));
            QObject::connect(m_dataset,SIGNAL(domainChanged(const Domain&)),item,SLOT(handleDomainChanged(const Domain&)));
            QObject::connect(percentBarSeries,SIGNAL(changed(int)),item,SLOT(handleModelChanged(int)));
            m_chartItems.insert(series,item);
            break;
        }
        /*
         case QChartSeries::SeriesTypeScatter: {
         QScatterSeries *scatterSeries = qobject_cast<QScatterSeries *>(series);
         scatterSeries->d->m_theme = m_chartTheme->themeForSeries();
         scatterSeries->d->setParentItem(this);
         scatterSeries->d->m_boundingRect = m_rect.adjusted(margin(),margin(), -margin(), -margin());
         m_chartItems << scatterSeries->d;
         m_chartTheme->addObserver(scatterSeries->d);

         foreach (qreal x, scatterSeries->d->m_x) {
         domain.m_minX = qMin(domain.m_minX, x);
         domain.m_maxX = qMax(domain.m_maxX, x);
         }
         foreach (qreal y, scatterSeries->d->m_y) {
         domain.m_minY = qMin(domain.m_minY, y);
         domain.m_maxY = qMax(domain.m_maxY, y);
         }

         break;
         }
         */

        case QChartSeries::SeriesTypePie: {
            QPieSeries *pieSeries = qobject_cast<QPieSeries *>(series);
            PiePresenter* pie = new PiePresenter(m_chart, pieSeries);
            pieSeries->m_piePresenter = pie; // TODO: remove this pointer passing use signals&slots
            QObject::connect(this, SIGNAL(geometryChanged(const QRectF&)), pie, SLOT(handleGeometryChanged(const QRectF&)));
            QObject::connect(m_dataset, SIGNAL(domainChanged(const Domain&)), pie, SLOT(handleDomainChanged(const Domain&)));
            m_chartItems.insert(series, pie);
            break;
        }

        default: {
            qDebug()<< "Series type" << series->type() << "not implemented.";
            break;
        }
    }

    if(m_rect.isValid()) emit geometryChanged(m_rect);
}

void ChartPresenter::handleSeriesChanged(QChartSeries* series)
{
    //TODO:
}

void ChartPresenter::zoomInToRect(const QRectF& rect)
{
    if(!rect.isValid()) return;
    QRectF r = rect.normalized();
    r.translate(-m_marginSize, -m_marginSize);
    Domain domain (m_dataset->domain().subDomain(r,m_rect.width(),m_rect.height()));
    m_dataset->addDomain(domain);
}

void ChartPresenter::zoomIn()
{
    if (!m_dataset->nextDomain()) {
        QRectF rect = m_rect;
        rect.setWidth(rect.width()/2);
        rect.setHeight(rect.height()/2);
        rect.moveCenter(m_rect.center());
        Domain domain (m_dataset->domain().subDomain(rect,m_rect.width(),m_rect.height()));
        m_dataset->addDomain(domain);
    }
}

void ChartPresenter::zoomOut()
{
    m_dataset->previousDomain();
}

void ChartPresenter::zoomReset()
{
    m_dataset->clearDomains();
}

void ChartPresenter::setChartTheme(QChart::ChartTheme theme)
{
    delete m_chartTheme;

    m_chartTheme = ChartTheme::createTheme(theme);

    m_chartTheme->decorate(m_chart);
    QMapIterator<QChartSeries*,ChartItem*> i(m_chartItems);

    int index=0;
    while (i.hasNext()) {
        i.next();
        index++;
        m_chartTheme->decorate(i.value(),i.key(),index);
      }
    }


QChart::ChartTheme ChartPresenter::chartTheme()
{
    return m_chartTheme->id();
}

void ChartPresenter::setAxisX(QChartAxis* axis)
{

}

void ChartPresenter::addAxisY(QChartAxis* axis)
{

}

/*
 void ChartPresenter::setAxisX(const QChartAxis& axis)
 {
 setAxis(m_axisXItem,axis);
 }
 void ChartPresenter::setAxisY(const QChartAxis& axis)
 {
 setAxis(m_axisYItem.at(0),axis);
 }

 void ChartPresenter::setAxisY(const QList<QChartAxis>& axis)
 {
 //TODO not implemented
 }

 void ChartPresenter::setAxis(AxisItem *item, const QChartAxis& axis)
 {
 item->setVisible(axis.isAxisVisible());
 }
 */
#include "moc_chartpresenter_p.cpp"

QTCOMMERCIALCHART_END_NAMESPACE
