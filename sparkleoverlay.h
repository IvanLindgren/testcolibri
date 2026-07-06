#ifndef SPARKLEOVERLAY_H
#define SPARKLEOVERLAY_H

#include <QColor>
#include <QTimer>
#include <QVector>
#include <QWidget>

class SparkleOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit SparkleOverlay(QWidget *parent);

    void setSparkleColor(const QColor &color);

protected:
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    struct Spark {
        double rx = 0.0;
        double ry = 0.0;
        double phase = 0.0;
        double speed = 1.0;
        double size = 5.0;
        int color = 0;
    };

    void regenerate();
    void relocate(Spark &spark);

    QVector<Spark> m_sparks;
    QColor m_color{Qt::white};
    QTimer m_timer;
    double m_t = 0.0;
};

#endif
