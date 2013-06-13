/****************************************************************************
**
** Copyright (C) 2013 Digia Plc
** All rights reserved.
** For any questions to Digia, please use contact form at http://qt.digia.com
**
** This file is part of the Qt Enterprise Charts Add-on.
**
** $QT_BEGIN_LICENSE$
** Licensees holding valid Qt Enterprise licenses may use this file in
** accordance with the Qt Enterprise License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.
**
** If you have questions regarding the use of this file, please use
** contact form at http://qt.digia.com
** $QT_END_LICENSE$
**
****************************************************************************/
#include "qabstractseries_p.h"
#include "qabstractaxis_p.h"
#include <QTime>
//themes
#include "chartthemesystem_p.h"
#include "chartthemelight_p.h"
#include "chartthemebluecerulean_p.h"
#include "chartthemedark_p.h"
#include "chartthemebrownsand_p.h"
#include "chartthemebluencs_p.h"
#include "chartthemehighcontrast_p.h"
#include "chartthemeblueicy_p.h"

QTCOMMERCIALCHART_BEGIN_NAMESPACE

ChartThemeManager::ChartThemeManager(QChart* chart) :
    m_chart(chart)
{
    qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));
}


void ChartThemeManager::setTheme(QChart::ChartTheme theme)
{
    if (m_theme.isNull() || theme != m_theme->id()) {
        switch (theme) {
        case QChart::ChartThemeLight:
            m_theme.reset(new ChartThemeLight());
            break;
        case QChart::ChartThemeBlueCerulean:
            m_theme.reset(new ChartThemeBlueCerulean());
            break;
        case QChart::ChartThemeDark:
            m_theme.reset(new ChartThemeDark());
            break;
        case QChart::ChartThemeBrownSand:
            m_theme.reset(new ChartThemeBrownSand());
            break;
        case QChart::ChartThemeBlueNcs:
            m_theme.reset(new ChartThemeBlueNcs());
            break;
        case QChart::ChartThemeHighContrast:
            m_theme.reset(new ChartThemeHighContrast());
            break;
        case QChart::ChartThemeBlueIcy:
            m_theme.reset(new ChartThemeBlueIcy());
            break;
        default:
            m_theme.reset(new ChartThemeSystem());
            break;
        }

        if (!m_theme.isNull()) {
            decorateChart(m_chart,m_theme.data());
            decorateLegend(m_chart->legend(),m_theme.data());
            foreach (QAbstractAxis* axis, m_axisList)
                axis->d_ptr->initializeTheme(m_theme.data(), true);
            foreach (QAbstractSeries* series, m_seriesMap.keys())
                series->d_ptr->initializeTheme(m_seriesMap[series], m_theme.data(), true);
        }
    }
}

// decorateChart is only called when theme is forcibly initialized
void ChartThemeManager::decorateChart(QChart *chart, ChartTheme *theme) const
{
    chart->setBackgroundBrush(theme->chartBackgroundGradient());

    QPen pen(Qt::transparent);
    QBrush brush;
    chart->setPlotAreaBackgroundBrush(brush);
    chart->setPlotAreaBackgroundPen(pen);
    chart->setPlotAreaBackgroundVisible(false);

    chart->setTitleFont(theme->masterFont());
    chart->setTitleBrush(theme->labelBrush());
    chart->setDropShadowEnabled(theme->isBackgroundDropShadowEnabled());
}

// decorateLegend is only called when theme is forcibly initialized
void ChartThemeManager::decorateLegend(QLegend *legend, ChartTheme *theme) const
{
    legend->setPen(theme->axisLinePen());
    legend->setBrush(theme->chartBackgroundGradient());
    legend->setFont(theme->labelFont());
    legend->setLabelBrush(theme->labelBrush());
}

int ChartThemeManager::createIndexKey(QList<int> keys) const
{
    qSort(keys);

    int key = 0;
    QList<int>::iterator i;
    i = keys.begin();

    while (i != keys.end()) {
        if (*i != key)
            break;
        key++;
        i++;
    }

    return key;
}

int ChartThemeManager::seriesCount(QAbstractSeries::SeriesType type) const
{
    int count = 0;
    QList<QAbstractSeries *> series =   m_seriesMap.keys();
    foreach(QAbstractSeries *s, series) {
        if (s->type() == type)
            count++;
    }
    return count;
}

void ChartThemeManager::handleSeriesAdded(QAbstractSeries *series)
{
    int key = createIndexKey(m_seriesMap.values());
    m_seriesMap.insert(series,key);
    series->d_ptr->initializeTheme(key,m_theme.data(),false);
}

void ChartThemeManager::handleSeriesRemoved(QAbstractSeries *series)
{
    m_seriesMap.remove(series);
}

void ChartThemeManager::handleAxisAdded(QAbstractAxis *axis)
{
    m_axisList.append(axis);
    axis->d_ptr->initializeTheme(m_theme.data(),false);
}

void ChartThemeManager::handleAxisRemoved(QAbstractAxis *axis)
{
    m_axisList.removeAll(axis);
}

void ChartThemeManager::updateSeries(QAbstractSeries *series)
{
    if(m_seriesMap.contains(series)){
        series->d_ptr->initializeTheme(m_seriesMap[series],m_theme.data(),false);
    }
}
QList<QGradient> ChartThemeManager::generateSeriesGradients(const QList<QColor>& colors)
{
    QList<QGradient> result;
    // Generate gradients in HSV color space
    foreach (const QColor &color, colors) {
        QLinearGradient g;
        qreal h = color.hsvHueF();
        qreal s = color.hsvSaturationF();

        QColor start = color;
        start.setHsvF(h, 0.0, 1.0);
        g.setColorAt(0.0, start);

        g.setColorAt(0.5, color);

        QColor end = color;
        end.setHsvF(h, s, 0.25);
        g.setColorAt(1.0, end);

        result << g;
    }

    return result;
}


QColor ChartThemeManager::colorAt(const QColor &start, const QColor &end, qreal pos)
{
    Q_ASSERT(pos >= 0.0 && pos <= 1.0);
    qreal r = start.redF() + ((end.redF() - start.redF()) * pos);
    qreal g = start.greenF() + ((end.greenF() - start.greenF()) * pos);
    qreal b = start.blueF() + ((end.blueF() - start.blueF()) * pos);
    QColor c;
    c.setRgbF(r, g, b);
    return c;
}

QColor ChartThemeManager::colorAt(const QGradient &gradient, qreal pos)
{
    Q_ASSERT(pos >= 0 && pos <= 1.0);

    QGradientStops stops = gradient.stops();
    int count = stops.count();

    // find previous stop relative to position
    QGradientStop prev = stops.first();
    for (int i = 0; i < count; i++) {
        QGradientStop stop = stops.at(i);
        if (pos > stop.first)
            prev = stop;

        // given position is actually a stop position?
        if (pos == stop.first) {
            //qDebug() << "stop color" << pos;
            return stop.second;
        }
    }

    // find next stop relative to position
    QGradientStop next = stops.last();
    for (int i = count - 1; i >= 0; i--) {
        QGradientStop stop = stops.at(i);
        if (pos < stop.first)
            next = stop;
    }

    //qDebug() << "prev" << prev.first << "pos" << pos << "next" << next.first;

    qreal range = next.first - prev.first;
    qreal posDelta = pos - prev.first;
    qreal relativePos = posDelta / range;

    //qDebug() << "range" << range << "posDelta" << posDelta << "relativePos" << relativePos;

    return colorAt(prev.second, next.second, relativePos);
}

#include "moc_chartthememanager_p.cpp"

QTCOMMERCIALCHART_END_NAMESPACE
