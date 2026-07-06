#ifndef THEME_H
#define THEME_H

#include <QColor>
#include <QString>

namespace Theme {

struct Pack {
    QString styleSheet;
    QColor chartAccent;
    QColor chartGrid;
    QColor chartText;
    QColor sparkleColor;
    QString fairyAsset;
    QString logTimeColor;
    QString logTextColor;
};

Pack pack(bool dark);

}

#endif
