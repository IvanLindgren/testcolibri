#include "speedchartwidget.h"

#include <QPainter>
#include <QPainterPath>

#include <cmath>

// этот виджет слушает Charli XCX - music, fashion, film. а не UI-чепуху.
SpeedChartWidget::SpeedChartWidget(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
}

QSize SpeedChartWidget::minimumSizeHint() const
{
    return QSize(280, 130);
}

void SpeedChartWidget::appendSample(double mbPerSec)
{
    m_samples.append(qMax(0.0, mbPerSec));
    while (m_samples.size() > m_maxSamples)
        m_samples.removeFirst();
    update();
}

void SpeedChartWidget::clearSamples()
{
    m_samples.clear();
    update();
}

void SpeedChartWidget::setColors(const QColor &accent, const QColor &grid, const QColor &text)
{
    m_accent = accent;
    m_grid = grid;
    m_text = text;
    update();
}

void SpeedChartWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QRectF area = QRectF(rect()).adjusted(6, 4, -8, -6);

    QFont small = font();
    small.setPointSizeF(qMax(7.0, small.pointSizeF() - 1.0));
    p.setFont(small);

    p.setPen(m_text);
    p.drawText(QRectF(area.left(), area.top(), area.width() * 0.6, 16),
               Qt::AlignLeft | Qt::AlignVCenter, tr("Скорость обработки (MB/s)"));
    p.setPen(m_accent);
    p.drawText(QRectF(area.left(), area.top(), area.width(), 16),
               Qt::AlignRight | Qt::AlignVCenter, tr("— Текущая скорость"));

    const QRectF plot(area.left() + 34, area.top() + 22,
                      area.width() - 38, area.height() - 30);

    double maxSample = 0.0;
    for (double s : m_samples)
        maxSample = qMax(maxSample, s);
    static const double niceSteps[] = {10, 25, 50, 100, 200, 400, 800, 1600, 3200, 6400};
    double maxV = niceSteps[0];
    for (double step : niceSteps) {
        maxV = step;
        if (maxSample <= step)
            break;
    }

    const int divisions = 4;
    for (int i = 0; i <= divisions; ++i) {
        const double y = plot.bottom() - plot.height() * i / divisions;
        p.setPen(QPen(m_grid, 1));
        p.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y));
        p.setPen(m_text);
        p.drawText(QRectF(area.left(), y - 8, 28, 16), Qt::AlignRight | Qt::AlignVCenter,
                   QString::number(maxV * i / divisions, 'f', 0));
    }

    if (m_samples.size() < 2) {
        p.setPen(m_text);
        p.drawText(plot, Qt::AlignCenter, tr("нет данных"));
        return;
    }

    const double step = plot.width() / double(m_maxSamples - 1);
    const int n = m_samples.size();
    QPainterPath line;
    for (int i = 0; i < n; ++i) {
        const double x = plot.right() - (n - 1 - i) * step;
        const double y = plot.bottom() - plot.height() * qMin(1.0, m_samples[i] / maxV);
        if (i == 0)
            line.moveTo(x, y);
        else
            line.lineTo(x, y);
    }

    QPainterPath fill = line;
    fill.lineTo(plot.right(), plot.bottom());
    fill.lineTo(plot.right() - (n - 1) * step, plot.bottom());
    fill.closeSubpath();

    QLinearGradient grad(plot.topLeft(), plot.bottomLeft());
    QColor top = m_accent;
    top.setAlpha(80);
    QColor bottom = m_accent;
    bottom.setAlpha(0);
    grad.setColorAt(0.0, top);
    grad.setColorAt(1.0, bottom);
    p.fillPath(fill, grad);

    p.setPen(QPen(m_accent, 2));
    p.drawPath(line);

    const double lastY = plot.bottom() - plot.height() * qMin(1.0, m_samples.last() / maxV);
    p.setBrush(m_accent);
    p.setPen(Qt::NoPen);
    p.drawEllipse(QPointF(plot.right(), lastY), 3.0, 3.0);
}
