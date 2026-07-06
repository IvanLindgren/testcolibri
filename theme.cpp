#include "theme.h"

namespace Theme {

static QString lightStyleSheet()
{
    return QStringLiteral(R"(
QMainWindow { background-color: #fdf4f7; }
QWidget#centralwidget { border-image: url(:/assets/light/background.jpg) 0 0 0 0 stretch stretch; }
QDialog, QMessageBox { background-color: #fdf4f7; }
QGroupBox {
    background-color: rgba(255, 255, 255, 242);
    border: 1px solid #f1d8e2;
    border-radius: 10px;
    margin-top: 0px;
    padding-top: 18px;
    font-weight: 600;
    color: #a94f74;
}
QGroupBox::title {
    subcontrol-origin: border;
    subcontrol-position: top left;
    left: 12px;
    top: 6px;
    padding: 0 2px;
    background-color: transparent;
}
QLabel { color: #503a46; background: transparent; }
QLabel#labelHint { color: #b18d9e; }
QLabel#labelControl { font-weight: 600; color: #a94f74; }
QLabel#labelDetName, QLabel#labelDetPath, QLabel#labelDetSize, QLabel#labelDetProcessed,
QLabel#labelDetSpeed, QLabel#labelDetElapsed, QLabel#labelDetRemaining, QLabel#labelDetStatus {
    font-weight: 600;
    color: #3d2a35;
}
QLineEdit, QComboBox, QSpinBox {
    background-color: #ffffff;
    border: 1px solid #e5bccd;
    border-radius: 6px;
    padding: 4px 8px;
    color: #3d2a35;
    selection-background-color: #e8aac4;
    selection-color: #ffffff;
}
QLineEdit:focus, QComboBox:focus, QSpinBox:focus { border: 1px solid #d67a9e; }
QLineEdit:disabled, QComboBox:disabled, QSpinBox:disabled { background-color: #faf3f6; color: #b39aa6; }
QComboBox::drop-down { border: none; width: 24px; }
QComboBox QAbstractItemView {
    background-color: #ffffff;
    border: 1px solid #e5bccd;
    selection-background-color: #f6dbe6;
    selection-color: #3d2a35;
    color: #3d2a35;
}
QCheckBox, QRadioButton { color: #503a46; background: transparent; spacing: 6px; }
QCheckBox:disabled, QRadioButton:disabled { color: #b39aa6; }
QCheckBox::indicator, QRadioButton::indicator {
    width: 14px;
    height: 14px;
    border: 2px solid #d9a4bb;
    background-color: #ffffff;
}
QCheckBox::indicator { border-radius: 4px; }
QRadioButton::indicator { border-radius: 9px; }
QCheckBox::indicator:hover, QRadioButton::indicator:hover { border-color: #c97ba0; }
QCheckBox::indicator:checked { background-color: #d76d97; border-color: #c25381; }
QRadioButton::indicator:checked {
    background-color: qradialgradient(cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5,
        stop:0 #d76d97, stop:0.55 #d76d97, stop:0.7 #ffffff, stop:1 #ffffff);
    border-color: #c25381;
}
QCheckBox::indicator:disabled, QRadioButton::indicator:disabled { border-color: #ecd5de; background-color: #f8eff3; }
QPushButton {
    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffffff, stop:1 #faecf2);
    border: 1px solid #e5bccd;
    border-radius: 6px;
    padding: 6px 12px;
    color: #8a4d66;
}
QPushButton:hover { background-color: #fbeef3; }
QPushButton:pressed { background-color: #f6dbe6; }
QPushButton:disabled { color: #cdb3bf; border-color: #f0dfe7; background-color: #fbf5f8; }
QPushButton#btnStart {
    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ea8bb4, stop:1 #d05f8d);
    border: none; color: #ffffff; font-weight: 600;
}
QPushButton#btnStart:hover { background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #e27ba7, stop:1 #c25381); }
QPushButton#btnStart:pressed { background-color: #b95179; }
QPushButton#btnStart:disabled { background-color: #eec9d8; color: #fdf4f7; }
QPushButton#btnPause {
    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #faeed9, stop:1 #f1ddb6);
    border: 1px solid #e9cf9f; color: #7d5f2b;
}
QPushButton#btnPause:hover { background-color: #f3e2c2; }
QPushButton#btnPause:disabled { background-color: #faf4ea; color: #d6c5a8; border-color: #f0e6d4; }
QPushButton#btnResume {
    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f9e4ed, stop:1 #f0cddd);
    border: 1px solid #e2a9c0; color: #a04a6d;
}
QPushButton#btnResume:hover { background-color: #f0cddd; }
QPushButton#btnResume:disabled { background-color: #faf0f4; color: #d9bcc9; border-color: #f2dfe8; }
QPushButton#btnStop {
    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f8dfdc, stop:1 #eec3be);
    border: 1px solid #e0a49e; color: #96453d;
}
QPushButton#btnStop:hover { background-color: #efc5c0; }
QPushButton#btnStop:disabled { background-color: #faeeec; color: #dcbcb8; border-color: #f2e0de; }
QProgressBar {
    background-color: #f7e6ec;
    border: none;
    border-radius: 7px;
    text-align: center;
    color: #6d4257;
}
QProgressBar::chunk {
    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ea8bb4, stop:1 #cf5c8b);
    border-radius: 7px;
}
QTableWidget {
    background-color: rgba(255, 255, 255, 235);
    alternate-background-color: rgba(252, 240, 246, 235);
    border: none;
    color: #3d2a35;
    selection-background-color: #f6dbe6;
    selection-color: #3d2a35;
}
QHeaderView::section {
    background-color: #fbeef3;
    color: #8a4d66;
    border: none;
    border-bottom: 1px solid #f1d8e2;
    padding: 6px 8px;
    font-weight: 600;
}
QTableWidget::item { padding: 2px 6px; }
QTextEdit {
    background-color: rgba(255, 251, 253, 240);
    border: 1px solid #f1d8e2;
    border-radius: 6px;
    color: #3d2a35;
    font-family: 'Consolas','Courier New',monospace;
}
QStatusBar { background-color: #fbeef3; border-top: 1px solid #f1d8e2; }
QStatusBar QLabel { color: #6d4257; }
QStatusBar::item { border: none; }
QScrollBar:vertical { background: transparent; width: 10px; margin: 0; }
QScrollBar::handle:vertical { background: #e8c3d3; border-radius: 5px; min-height: 30px; }
QScrollBar::handle:vertical:hover { background: #d9a8bf; }
QScrollBar:horizontal { background: transparent; height: 10px; margin: 0; }
QScrollBar::handle:horizontal { background: #e8c3d3; border-radius: 5px; min-width: 30px; }
QScrollBar::add-line, QScrollBar::sub-line { width: 0; height: 0; }
QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }
QToolTip { background-color: #ffffff; color: #503a46; border: 1px solid #e5bccd; }
)");
}

static QString darkStyleSheet()
{
    return QStringLiteral(R"(
QMainWindow { background-color: #241a21; }
QWidget#centralwidget { border-image: url(:/assets/dark/background.jpg) 0 0 0 0 stretch stretch; }
QDialog, QMessageBox { background-color: #2b1f27; }
QGroupBox {
    background-color: rgba(38, 26, 33, 248);
    border: 1px solid #553b48;
    border-radius: 10px;
    margin-top: 0px;
    padding-top: 18px;
    font-weight: 600;
    color: #f2a9cc;
}
QGroupBox::title {
    subcontrol-origin: border;
    subcontrol-position: top left;
    left: 12px;
    top: 6px;
    padding: 0 2px;
    background-color: transparent;
}
QLabel { color: #f0e2e9; background: transparent; }
QLabel#labelHint { color: #ab8c9b; }
QLabel#labelControl { font-weight: 600; color: #f2a9cc; }
QLabel#labelDetName, QLabel#labelDetPath, QLabel#labelDetSize, QLabel#labelDetProcessed,
QLabel#labelDetSpeed, QLabel#labelDetElapsed, QLabel#labelDetRemaining, QLabel#labelDetStatus {
    font-weight: 600;
    color: #ffffff;
}
QLineEdit, QComboBox, QSpinBox {
    background-color: rgba(28, 19, 24, 250);
    border: 1px solid #6a4a5c;
    border-radius: 6px;
    padding: 4px 8px;
    color: #fbf2f6;
    selection-background-color: #a35577;
    selection-color: #ffffff;
}
QLineEdit:focus, QComboBox:focus, QSpinBox:focus { border: 1px solid #e389af; }
QLineEdit:disabled, QComboBox:disabled, QSpinBox:disabled { background-color: rgba(40, 29, 36, 220); color: #8d7280; }
QComboBox::drop-down { border: none; width: 24px; }
QComboBox QAbstractItemView {
    background-color: #2f212b;
    border: 1px solid #6a4a5c;
    selection-background-color: #5c3f4e;
    selection-color: #ffffff;
    color: #f0e2e9;
}
QCheckBox, QRadioButton { color: #f0e2e9; background: transparent; spacing: 6px; }
QCheckBox:disabled, QRadioButton:disabled { color: #8d7280; }
QCheckBox::indicator, QRadioButton::indicator {
    width: 14px;
    height: 14px;
    border: 2px solid #a5718b;
    background-color: #1e1419;
}
QCheckBox::indicator { border-radius: 4px; }
QRadioButton::indicator { border-radius: 9px; }
QCheckBox::indicator:hover, QRadioButton::indicator:hover { border-color: #d488ab; }
QCheckBox::indicator:checked { background-color: #e07aa5; border-color: #efa4c4; }
QRadioButton::indicator:checked {
    background-color: qradialgradient(cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5,
        stop:0 #e07aa5, stop:0.55 #e07aa5, stop:0.7 #1e1419, stop:1 #1e1419);
    border-color: #efa4c4;
}
QCheckBox::indicator:disabled, QRadioButton::indicator:disabled { border-color: #55404b; background-color: #2b1f27; }
QPushButton {
    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #47323d, stop:1 #382730);
    border: 1px solid #7a5568;
    border-radius: 6px;
    padding: 6px 12px;
    color: #f5cede;
}
QPushButton:hover { background-color: #533b47; }
QPushButton:pressed { background-color: #5c414f; }
QPushButton:disabled { color: #7c6370; border-color: #4f3b46; background-color: rgba(46, 33, 41, 200); }
QPushButton#btnStart {
    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #e07aa5, stop:1 #b94f7c);
    border: none; color: #ffffff; font-weight: 600;
}
QPushButton#btnStart:hover { background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #d46e99, stop:1 #ad4671); }
QPushButton#btnStart:pressed { background-color: #a04168; }
QPushButton#btnStart:disabled { background-color: #5c3f4e; color: #937686; }
QPushButton#btnPause {
    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #5f4c33, stop:1 #4c3c22);
    border: 1px solid #a5854c; color: #f5d99b;
}
QPushButton#btnPause:hover { background-color: #64502f; }
QPushButton#btnPause:disabled { background-color: #3a3126; color: #7d7057; border-color: #55492f; }
QPushButton#btnResume {
    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #5f4252, stop:1 #4c3440);
    border: 1px solid #b56d90; color: #f7c6da;
}
QPushButton#btnResume:hover { background-color: #684757; }
QPushButton#btnResume:disabled { background-color: #3a2c34; color: #7c6370; border-color: #4f3b46; }
QPushButton#btnStop {
    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #613737, stop:1 #4e2929);
    border: 1px solid #a05b55; color: #f5b3ab;
}
QPushButton#btnStop:hover { background-color: #683b3b; }
QPushButton#btnStop:disabled { background-color: #3d2b2b; color: #7c6060; border-color: #543c3a; }
QProgressBar {
    background-color: rgba(64, 45, 56, 250);
    border: none;
    border-radius: 7px;
    text-align: center;
    color: #f0e2e9;
}
QProgressBar::chunk {
    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #e07aa5, stop:1 #c05580);
    border-radius: 7px;
}
QTableWidget {
    background-color: rgba(30, 21, 26, 242);
    alternate-background-color: rgba(41, 29, 36, 242);
    border: none;
    color: #f0e2e9;
    selection-background-color: #5c3f4e;
    selection-color: #ffffff;
}
QHeaderView::section {
    background-color: rgba(64, 45, 56, 250);
    color: #f2b3d1;
    border: none;
    border-bottom: 1px solid #553b48;
    padding: 6px 8px;
    font-weight: 600;
}
QTableWidget::item { padding: 2px 6px; }
QTextEdit {
    background-color: rgba(24, 16, 21, 245);
    border: 1px solid #553b48;
    border-radius: 6px;
    color: #f0e2e9;
    font-family: 'Consolas','Courier New',monospace;
}
QStatusBar { background-color: #322230; border-top: 1px solid #553b48; }
QStatusBar QLabel { color: #e9c6d7; }
QStatusBar::item { border: none; }
QScrollBar:vertical { background: transparent; width: 10px; margin: 0; }
QScrollBar::handle:vertical { background: #6a4a5c; border-radius: 5px; min-height: 30px; }
QScrollBar::handle:vertical:hover { background: #83596f; }
QScrollBar:horizontal { background: transparent; height: 10px; margin: 0; }
QScrollBar::handle:horizontal { background: #6a4a5c; border-radius: 5px; min-width: 30px; }
QScrollBar::add-line, QScrollBar::sub-line { width: 0; height: 0; }
QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }
QToolTip { background-color: #2f212b; color: #f0e2e9; border: 1px solid #7a5568; }
)");
}

Pack pack(bool dark)
{
    Pack p;
    if (dark) {
        p.styleSheet = darkStyleSheet();
        p.chartAccent = QColor(0xe8, 0x8c, 0xb2);
        p.chartGrid = QColor(0x55, 0x3b, 0x48);
        p.chartText = QColor(0xe9, 0xc6, 0xd7);
        p.sparkleColor = QColor(Qt::white);
        p.fairyAsset = QStringLiteral(":/assets/dark/musa.png");
        p.logTimeColor = QStringLiteral("#ab8c9b");
        p.logTextColor = QStringLiteral("#f0e2e9");
    } else {
        p.styleSheet = lightStyleSheet();
        p.chartAccent = QColor(0xd7, 0x6d, 0x97);
        p.chartGrid = QColor(0xf0, 0xdb, 0xe4);
        p.chartText = QColor(0x8a, 0x5a, 0x70);
        p.sparkleColor = QColor(0xf2, 0x8b, 0xb8);
        p.fairyAsset = QStringLiteral(":/assets/light/flora_no.png");
        p.logTimeColor = QStringLiteral("#bb93a4");
        p.logTextColor = QStringLiteral("#4b3543");
    }
    return p;
}

}
