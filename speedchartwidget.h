#ifndef SPEEDCHARTWIDGET_H
#define SPEEDCHARTWIDGET_H

#include <QColor>
#include <QVector>
#include <QWidget>

class SpeedChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SpeedChartWidget(QWidget *parent = nullptr);

    void appendSample(double mbPerSec);
    void clearSamples();
    void setColors(const QColor &accent, const QColor &grid, const QColor &text);

    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<double> m_samples;
    int m_maxSamples = 180;
    QColor m_accent{0xd7, 0x6d, 0x97};
    QColor m_grid{0xf0, 0xdb, 0xe4};
    QColor m_text{0x8a, 0x5a, 0x70};
};

#endif
