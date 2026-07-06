#include "sparkleoverlay.h"

#include <QEvent>
#include <QPainter>
#include <QRandomGenerator>

#include <cmath>

namespace {
constexpr int kSparkCount = 22;
constexpr int kTickMs = 130;
constexpr double kStripMaxHeight = 90.0;
}

//зачем вы сюда зашли, это же просто метод отрисовки блесток, не надо сюда лезть, там ничего интересного нет, идите лучше в theme.cpp и посмотрите на цвета, там красиво.
SparkleOverlay::SparkleOverlay(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    parent->installEventFilter(this);
    setGeometry(parent->rect());
    regenerate();

    m_timer.setInterval(kTickMs);
    connect(&m_timer, &QTimer::timeout, this, [this]() {
        m_t += kTickMs / 1000.0;
        for (Spark &s : m_sparks) {
            const double a = 0.5 * (1.0 + std::sin(m_t * s.speed + s.phase));
            if (a < 0.15 && QRandomGenerator::global()->bounded(100) < 20)
                relocate(s);
        }
        update();
    });
    m_timer.start();
    raise();
}

void SparkleOverlay::setSparkleColor(const QColor &color)
{
    m_color = color;
    update();
}

void SparkleOverlay::regenerate()
{
    m_sparks.clear();
    auto *rng = QRandomGenerator::global();
    for (int i = 0; i < kSparkCount; ++i) {
        Spark s;
        relocate(s);
        s.phase = rng->generateDouble() * 6.283;
        s.speed = 1.2 + rng->generateDouble() * 2.4;
        s.size = 3.0 + rng->generateDouble() * 5.0;
        s.color = int(rng->bounded(1000));
        m_sparks.append(s);
    }
}

void SparkleOverlay::relocate(Spark &spark)
{
    auto *rng = QRandomGenerator::global();
    spark.rx = rng->generateDouble();
    spark.ry = rng->generateDouble();
}

bool SparkleOverlay::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parent()
        && (event->type() == QEvent::Resize || event->type() == QEvent::Show)) {
        setGeometry(parentWidget()->rect());
        raise();
    }
    return QWidget::eventFilter(watched, event);
}

void SparkleOverlay::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    const double stripHeight = qMin(kStripMaxHeight, height() * 0.15);

    for (const Spark &s : m_sparks) {
        double a = 0.5 * (1.0 + std::sin(m_t * s.speed + s.phase));
        a = std::floor(a * 4.0) / 3.0;
        if (a <= 0.0)
            continue;
        a = qMin(a, 1.0);

        const int x = int(s.rx * width());
        const int y = int(s.ry * stripHeight);
        const int len = int(s.size);
        const int diag = qMax(1, int(s.size * 0.45));

        QColor c = m_color;

        c.setAlphaF(a * 0.95);
        p.setPen(QPen(c, 1));
        p.drawLine(x - len, y, x + len, y);
        p.drawLine(x, y - len, x, y + len);

        c.setAlphaF(a * 0.55);
        p.setPen(QPen(c, 1));
        p.drawLine(x - diag, y - diag, x + diag, y + diag);
        p.drawLine(x - diag, y + diag, x + diag, y - diag);

        c.setAlphaF(a);
        p.fillRect(x - 1, y - 1, 2, 2, c);
    }
}
