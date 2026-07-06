#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "fileprocessor.h"
#include "sparkleoverlay.h"
#include "theme.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QLabel>
#include <QLocale>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QTableWidgetItem>
#include <QTime>
#include <QVariant>

namespace {

enum Column {
    ColName = 0,
    ColSize,
    ColStatus,
    ColProgress,
    ColSpeed,
    ColElapsed,
    ColRemaining,
    ColumnCount
};

const QColor kColorQueued(0xcf, 0x7f, 0x2f);
const QColor kColorRunning(0xc9, 0x62, 0x8d);
const QColor kColorPaused(0xcf, 0x7f, 0x2f);
const QColor kColorDone(0x35, 0xab, 0x63);
const QColor kColorError(0xd0, 0x4a, 0x3a);
const QColor kColorCancelled(0x98, 0x85, 0x8e);
constexpr int kProgressRole = Qt::UserRole + 1;

QColor widgetColorProperty(const QWidget *widget, const char *name, const QColor &fallback)
{
    const QVariant value = widget ? widget->property(name) : QVariant();
    return value.canConvert<QColor>() ? value.value<QColor>() : fallback;
}

// этот делегат слушает Madonna - Confession II. а не UI-чепуху.
class ProgressDelegate final : public QStyledItemDelegate
{
public:
    explicit ProgressDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        QStyleOptionViewItem itemOption(option);
        initStyleOption(&itemOption, index);
        itemOption.text.clear();

        const QWidget *widget = option.widget;
        QStyle *style = widget ? widget->style() : QApplication::style();
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &itemOption, painter, widget);

        const int percent = qBound(0, index.data(kProgressRole).toInt(), 100);
        const QRectF bar = option.rect.adjusted(6, 8, -6, -8);
        const QColor track = widgetColorProperty(widget, "progressTrack", option.palette.base().color());
        const QColor accent = widgetColorProperty(widget, "progressAccent", option.palette.highlight().color());
        const QColor text = widgetColorProperty(widget, "progressText", option.palette.text().color());

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(track);
        painter->drawRoundedRect(bar, 7.0, 7.0);

        QRectF fill = bar;
        fill.setWidth(bar.width() * percent / 100.0);
        if (fill.width() > 0.0) {
            painter->save();
            painter->setClipRect(fill);
            painter->setBrush(accent);
            painter->drawRoundedRect(bar, 7.0, 7.0);
            painter->restore();
        }

        painter->setPen(text);
        painter->drawText(option.rect, Qt::AlignCenter, QStringLiteral("%1%").arg(percent));
        painter->restore();
    }
};

// ключ известного файла нормализуется без учёта регистра
QString pathKey(const QString &path)
{
    return QDir::cleanPath(QFileInfo(path).absoluteFilePath()).toLower();
}

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->lineEditHexKey->setValidator(
        new QRegularExpressionValidator(QRegularExpression(QStringLiteral("[0-9A-Fa-f]{0,16}")), this));

    ui->tableFiles->horizontalHeader()->setSectionResizeMode(ColName, QHeaderView::Stretch);
    ui->tableFiles->setColumnWidth(ColSize, 90);
    ui->tableFiles->setColumnWidth(ColStatus, 100);
    ui->tableFiles->setColumnWidth(ColProgress, 160);
    ui->tableFiles->setColumnWidth(ColSpeed, 90);
    ui->tableFiles->setColumnWidth(ColElapsed, 80);
    ui->tableFiles->setColumnWidth(ColRemaining, 80);
    ui->tableFiles->verticalHeader()->setDefaultSectionSize(34);
    ui->tableFiles->setItemDelegateForColumn(ColProgress, new ProgressDelegate(ui->tableFiles));

    m_statusLabel = new QLabel(this);
    m_modeLabel = new QLabel(this);
    m_uptimeLabel = new QLabel(this);
    statusBar()->addWidget(m_statusLabel, 1);
    statusBar()->addPermanentWidget(m_modeLabel);
    statusBar()->addPermanentWidget(m_uptimeLabel);
    m_modeLabel->setText(tr("Режим: —"));
    m_uptimeLabel->setText(tr("Время работы: 00:00:00"));

    // воркер живёт в отдельном потоке, что логично...
    m_processor = new FileProcessor;
    m_processor->moveToThread(&m_workerThread);
    connect(&m_workerThread, &QThread::finished, m_processor, &QObject::deleteLater);
    connect(this, &MainWindow::startFileRequested, m_processor, &FileProcessor::processFile);
    connect(m_processor, &FileProcessor::fileStarted, this, &MainWindow::onFileStarted);
    connect(m_processor, &FileProcessor::progressChanged, this, &MainWindow::onFileProgress);
    connect(m_processor, &FileProcessor::filePaused, this, &MainWindow::onFilePaused);
    connect(m_processor, &FileProcessor::fileResumed, this, &MainWindow::onFileResumed);
    connect(m_processor, &FileProcessor::fileFinished, this, &MainWindow::onFileFinished);
    m_workerThread.start();

    connect(ui->btnStart, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(ui->btnPause, &QPushButton::clicked, this, &MainWindow::onPauseClicked);
    connect(ui->btnResume, &QPushButton::clicked, this, &MainWindow::onResumeClicked);
    connect(ui->btnStop, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(ui->btnValidateKey, &QPushButton::clicked, this, &MainWindow::onValidateKeyClicked);
    connect(ui->btnBrowseInput, &QPushButton::clicked, this, &MainWindow::onBrowseInput);
    connect(ui->btnBrowseOutput, &QPushButton::clicked, this, &MainWindow::onBrowseOutput);
    connect(ui->btnClearLog, &QPushButton::clicked, this, &MainWindow::onClearLog);
    connect(ui->btnClearList, &QPushButton::clicked, this, &MainWindow::onClearList);
    connect(ui->btnTheme, &QPushButton::clicked, this, &MainWindow::onThemeToggle);

    connect(&m_pollTimer, &QTimer::timeout, this, &MainWindow::onPollTimer);
    m_uptimeTimer.setInterval(1000);
    connect(&m_uptimeTimer, &QTimer::timeout, this, &MainWindow::onUptimeTick);

    m_sparkles = new SparkleOverlay(ui->centralwidget);
    applyTheme(false);

    resetDetails();
    setState(State::Idle);
    logInfo(tr("Программа запущена"));
}

MainWindow::~MainWindow()
{
    shutdownWorker();
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // при закрытии будим возможную паузу и дожидаемся завершения рабочего потока.
    m_pollTimer.stop();
    shutdownWorker();
    event->accept();
}

void MainWindow::shutdownWorker()
{
    if (m_workerThread.isRunning()) {
        m_processor->requestStop();
        m_workerThread.quit();
        m_workerThread.wait();
    }
}

void MainWindow::onStartClicked()
{
    const QString inputDir = ui->lineEditInputDir->text().trimmed();
    if (inputDir.isEmpty() || !QDir(inputDir).exists()) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Укажите существующий путь для поиска входных файлов."));
        return;
    }

    const QString outputDir = ui->lineEditOutputDir->text().trimmed();
    if (outputDir.isEmpty()) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Укажите путь для сохранения результатов."));
        return;
    }
    if (!QDir().mkpath(outputDir)) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось создать папку результатов."));
        return;
    }

    quint64 key = 0;
    QString keyError;
    if (!parseKey(&key, &keyError)) {
        QMessageBox::warning(this, tr("Ошибка"), keyError);
        return;
    }

    QStringList masks;
    const QStringList rawMasks = ui->lineEditMask->text().split(QRegularExpression(QStringLiteral("[;,]")),
                                                                Qt::SkipEmptyParts);
    for (const QString &m : rawMasks) {
        const QString trimmed = m.trimmed();
        if (!trimmed.isEmpty())
            masks << trimmed;
    }
    if (masks.isEmpty()) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Укажите маску входных файлов, например *.bin;*.txt"));
        return;
    }

    m_inputDir = inputDir;
    m_outputDir = outputDir;
    m_key = key;
    m_masks = masks;
    m_deleteInput = ui->checkDeleteInput->isChecked();
    m_overwrite = ui->comboConflict->currentIndex() == 0;
    m_timerMode = ui->radioTimer->isChecked();

    resetSession();
    m_processor->resetControlFlags();
    setState(State::Running);

    m_uptime.start();
    m_uptimeTimer.start();
    onUptimeTick();

    if (m_timerMode) {
        const int factor = ui->comboIntervalUnit->currentIndex() == 1 ? 60000 : 1000;
        const int intervalMs = ui->spinInterval->value() * factor;
        m_pollTimer.start(intervalMs);
        m_modeLabel->setText(tr("Режим: По таймеру (%1 %2)")
                                 .arg(ui->spinInterval->value())
                                 .arg(ui->comboIntervalUnit->currentText()));
    } else {
        m_modeLabel->setText(tr("Режим: Разовый запуск"));
    }

    logInfo(tr("Обработка запущена"));
    logInfo(tr("Поиск файлов в: %1").arg(QDir::toNativeSeparators(m_inputDir)));

    scanInputDirectory();

    if (!m_timerMode && m_files.isEmpty()) {
        logWarn(tr("Файлы по заданной маске не найдены"));
        m_uptimeTimer.stop();
        setState(State::Idle);
        return;
    }

    dispatchNext();
}

void MainWindow::onPauseClicked()
{
    if (m_state != State::Running)
        return;
    m_processor->requestPause();
    setState(State::Paused);
    logInfo(tr("Обработка приостановлена пользователем"));
}

void MainWindow::onResumeClicked()
{
    if (m_state != State::Paused)
        return;
    m_processor->requestResume();
    setState(State::Running);
    logInfo(tr("Обработка возобновлена"));
    dispatchNext();
}

void MainWindow::onStopClicked()
{
    if (m_state == State::Idle)
        return;

    m_pollTimer.stop();
    m_processor->requestStop();

    for (int id : std::as_const(m_queue))
        setRowStatus(id, tr("Отменено"), kColorCancelled);
    m_queue.clear();

    m_uptimeTimer.stop();
    setState(State::Idle);
    logInfo(tr("Обработка остановлена пользователем"));
}

void MainWindow::onValidateKeyClicked()
{
    quint64 key = 0;
    QString error;
    if (parseKey(&key, &error)) {
        QString bytes;
        for (int i = 0; i < 8; ++i)
            bytes += QStringLiteral("%1 ").arg(quint8(key >> (56 - 8 * i)), 2, 16, QLatin1Char('0')).toUpper();
        logInfo(tr("Значение XOR корректно, ура!: %1").arg(bytes.trimmed()));
        QMessageBox::information(this, tr("Проверка значения"),
                                 tr("Значение корректно.\nБайты ключа: %1").arg(bytes.trimmed()));
    } else {
        logWarn(tr("Некорректное значение XOR, блин!: %1").arg(ui->lineEditHexKey->text()));
        QMessageBox::warning(this, tr("Проверка значения"), error);
    }
}

void MainWindow::onBrowseInput()
{
    const QString dir = QFileDialog::getExistingDirectory(this, tr("Путь для поиска входных файлов"),
                                                          ui->lineEditInputDir->text());
    if (!dir.isEmpty())
        ui->lineEditInputDir->setText(dir);
}

void MainWindow::onBrowseOutput()
{
    const QString dir = QFileDialog::getExistingDirectory(this, tr("Путь для сохранения результатов"),
                                                          ui->lineEditOutputDir->text());
    if (!dir.isEmpty())
        ui->lineEditOutputDir->setText(dir);
}

void MainWindow::onClearLog()
{
    m_logEntries.clear();
    ui->logView->clear();
}

void MainWindow::onClearList()
{
    if (m_state != State::Idle)
        return;
    resetSession();
}

void MainWindow::onThemeToggle()
{
    applyTheme(!m_darkTheme);
}

void MainWindow::applyTheme(bool dark)
{
    m_darkTheme = dark;
    const Theme::Pack pack = Theme::pack(dark);

    setStyleSheet(pack.styleSheet);
    ui->speedChart->setColors(pack.chartAccent, pack.chartGrid, pack.chartText);
    ui->tableFiles->setProperty("progressAccent", pack.chartAccent);
    ui->tableFiles->setProperty("progressTrack", dark ? QColor(0x40, 0x2d, 0x38) : QColor(0xf7, 0xe6, 0xec));
    ui->tableFiles->setProperty("progressText", pack.chartText);
    m_sparkles->setSparkleColor(pack.sparkleColor);
    ui->labelFairy->setPixmap(QPixmap(pack.fairyAsset)
                                  .scaledToHeight(150, Qt::SmoothTransformation));
    ui->btnTheme->setText(dark ? tr("Светлая тема") : tr("Тёмная тема"));

    m_logTimeColor = pack.logTimeColor;
    m_logTextColor = pack.logTextColor;
    rebuildLog();
}

void MainWindow::onPollTimer()
{
    if (m_state == State::Idle) {
        m_pollTimer.stop();
        return;
    }
    scanInputDirectory();
    if (m_state == State::Running)
        dispatchNext();
}

void MainWindow::onUptimeTick()
{
    m_uptimeLabel->setText(tr("Время работы: %1").arg(formatDuration(m_uptime.elapsed())));
}

void MainWindow::onFileStarted(int id, const QString &outputPath)
{
    // результат тоже помечаем известным, чтобы опрос по таймеру не подхватил
    // его как новый входной файл (если папки ввода и вывода совпадают).
    m_knownPaths.insert(pathKey(outputPath));

    const auto it = m_files.constFind(id);
    if (it == m_files.constEnd())
        return;

    setRowStatus(id, tr("Обработка"), kColorRunning);
    ui->speedChart->clearSamples();

    const QFileInfo fi(it->path);
    ui->labelDetName->setText(fi.fileName());
    ui->labelDetPath->setText(QDir::toNativeSeparators(it->path));
    ui->labelDetPath->setToolTip(ui->labelDetPath->text());
    ui->labelDetSize->setText(formatSizeFull(it->size));
    ui->labelDetStatus->setText(tr("Обработка"));
    ui->progressCurrent->setValue(0);

    logInfo(tr("Начата обработка: %1").arg(fi.fileName()));
}

void MainWindow::onFileProgress(int id, qint64 processed, qint64 total,
                                double bytesPerSecond, qint64 elapsedMs, qint64 remainingMs)
{
    const auto it = m_files.constFind(id);
    if (it == m_files.constEnd())
        return;

    const int percent = total > 0 ? int(processed * 100 / total) : 100;

    setProgressForRow(it->row, percent);
    ui->tableFiles->item(it->row, ColSpeed)->setText(formatSpeed(bytesPerSecond));
    ui->tableFiles->item(it->row, ColElapsed)->setText(formatDuration(elapsedMs));
    ui->tableFiles->item(it->row, ColRemaining)->setText(formatDuration(remainingMs));

    if (id == m_currentId) {
        ui->speedChart->appendSample(bytesPerSecond / double(1LL << 20));
        ui->labelDetProcessed->setText(formatSizeFull(processed));
        ui->labelDetSpeed->setText(formatSpeed(bytesPerSecond));
        ui->labelDetElapsed->setText(formatDuration(elapsedMs));
        ui->labelDetRemaining->setText(formatDuration(remainingMs));
        ui->progressCurrent->setValue(percent);
        m_currentFraction = total > 0 ? double(processed) / double(total) : 1.0;
        updateOverallProgress();
    }
}

void MainWindow::onFilePaused(int id)
{
    setRowStatus(id, tr("Пауза"), kColorPaused);
    if (id == m_currentId)
        ui->labelDetStatus->setText(tr("Пауза"));
}

void MainWindow::onFileResumed(int id)
{
    setRowStatus(id, tr("Обработка"), kColorRunning);
    if (id == m_currentId)
        ui->labelDetStatus->setText(tr("Обработка"));
}

void MainWindow::onFileFinished(int id, int result, const QString &message)
{
    const auto it = m_files.find(id);
    if (it == m_files.end())
        return;
    it->done = true;
    ++m_doneCount;

    const QString name = QFileInfo(it->path).fileName();

    switch (result) {
    case FileProcessor::Success:
        setRowStatus(id, tr("Завершено"), kColorDone);
        setProgressForRow(it->row, 100);
        ui->tableFiles->item(it->row, ColRemaining)->setText(QStringLiteral("00:00:00"));
        if (message.isEmpty())
            logInfo(tr("Завершена обработка: %1").arg(name));
        else
            logWarn(tr("Завершена обработка: %1 (%2)").arg(name, message));
        break;
    case FileProcessor::Cancelled:
        setRowStatus(id, tr("Остановлено"), kColorCancelled);
        logWarn(tr("Обработка прервана: %1").arg(name));
        break;
    default:
        setRowStatus(id, tr("Ошибка"), kColorError);
        logError(tr("Ошибка обработки %1: %2").arg(name, message));
        break;
    }

    if (id == m_currentId) {
        m_currentId = -1;
        m_currentFraction = 0.0;
        ui->labelDetStatus->setText(result == FileProcessor::Success ? tr("Завершено") : tr("—"));
    }

    updateCounters();
    updateOverallProgress();

    if (m_state == State::Running)
        dispatchNext();
}

void MainWindow::scanInputDirectory()
{
    int added = 0;
    const bool updatesEnabled = ui->tableFiles->updatesEnabled();
    ui->tableFiles->setUpdatesEnabled(false);

    QDirIterator it(m_inputDir, m_masks, QDir::Files | QDir::Readable);
    while (it.hasNext()) {
        it.next();
        const QFileInfo fi = it.fileInfo();
        if (fi.fileName().endsWith(QLatin1String(".xortmp"), Qt::CaseInsensitive))
            continue;
        const QString key = pathKey(fi.filePath());
        if (m_knownPaths.contains(key))
            continue;
        m_knownPaths.insert(key);

        const int id = m_nextId++;
        FileEntry entry;
        entry.path = fi.absoluteFilePath();
        entry.size = fi.size();
        m_files.insert(id, entry);
        addFileRow(id);
        m_queue.append(id);
        ++added;

        if (!m_firstScan)
            logInfo(tr("Найден новый файл: %1").arg(fi.fileName()));
    }

    ui->tableFiles->setUpdatesEnabled(updatesEnabled);

    if (m_firstScan) {
        logInfo(tr("Найдено файлов: %1").arg(added));
        m_firstScan = false;
    }

    if (added > 0) {
        updateCounters();
        updateOverallProgress();
    }
}

void MainWindow::dispatchNext()
{
    if (m_state != State::Running || m_currentId != -1)
        return;
    if (m_queue.isEmpty()) {
        finishSessionIfDone();
        return;
    }

    const int id = m_queue.takeFirst();
    m_currentId = id;
    const FileEntry &entry = m_files.value(id);
    emit startFileRequested(id, entry.path, m_outputDir, m_key, m_deleteInput, m_overwrite);
}

void MainWindow::finishSessionIfDone()
{
    if (m_timerMode) {
        m_statusLabel->setText(tr("Статус: <span style=\"color:#35ab63;\">Ожидание файлов</span>"));
        return;
    }
    m_uptimeTimer.stop();
    setState(State::Idle);
    logInfo(tr("Все файлы обработаны (%1 из %2)").arg(m_doneCount).arg(m_files.size()));
}

void MainWindow::addFileRow(int id)
{
    FileEntry &entry = m_files[id];
    const int row = ui->tableFiles->rowCount();
    ui->tableFiles->insertRow(row);
    entry.row = row;

    auto *nameItem = new QTableWidgetItem(QFileInfo(entry.path).fileName());
    nameItem->setToolTip(QDir::toNativeSeparators(entry.path));
    ui->tableFiles->setItem(row, ColName, nameItem);

    auto *sizeItem = new QTableWidgetItem(formatSize(entry.size));
    sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tableFiles->setItem(row, ColSize, sizeItem);

    auto *statusItem = new QTableWidgetItem(tr("В очереди"));
    statusItem->setForeground(kColorQueued);
    ui->tableFiles->setItem(row, ColStatus, statusItem);

    auto *progressItem = new QTableWidgetItem(QStringLiteral("0%"));
    progressItem->setData(kProgressRole, 0);
    progressItem->setTextAlignment(Qt::AlignCenter);
    ui->tableFiles->setItem(row, ColProgress, progressItem);

    auto *speedItem = new QTableWidgetItem(QStringLiteral("—"));
    speedItem->setTextAlignment(Qt::AlignCenter);
    ui->tableFiles->setItem(row, ColSpeed, speedItem);

    auto *elapsedItem = new QTableWidgetItem(QStringLiteral("00:00:00"));
    elapsedItem->setTextAlignment(Qt::AlignCenter);
    ui->tableFiles->setItem(row, ColElapsed, elapsedItem);

    auto *remainingItem = new QTableWidgetItem(QStringLiteral("—"));
    remainingItem->setTextAlignment(Qt::AlignCenter);
    ui->tableFiles->setItem(row, ColRemaining, remainingItem);
}

void MainWindow::setRowStatus(int id, const QString &text, const QColor &color)
{
    const auto it = m_files.constFind(id);
    if (it == m_files.constEnd())
        return;
    QTableWidgetItem *item = ui->tableFiles->item(it->row, ColStatus);
    if (item) {
        item->setText(text);
        item->setForeground(color);
    }
}

void MainWindow::setProgressForRow(int row, int percent)
{
    QTableWidgetItem *item = ui->tableFiles->item(row, ColProgress);
    if (!item)
        return;

    percent = qBound(0, percent, 100);
    if (item->data(kProgressRole).toInt() == percent)
        return;

    item->setData(kProgressRole, percent);
    item->setText(QStringLiteral("%1%").arg(percent));
}

void MainWindow::updateCounters()
{
    ui->labelFilesProcessed->setText(tr("Файлов обработано: %1 из %2")
                                         .arg(m_doneCount)
                                         .arg(m_files.size()));
}

void MainWindow::updateOverallProgress()
{
    const int total = m_files.size();
    const int percent = total > 0
            ? int((m_doneCount + m_currentFraction) * 100.0 / total)
            : 0;
    ui->progressTotal->setValue(qBound(0, percent, 100));
}

void MainWindow::setState(State state)
{
    m_state = state;
    const bool idle = state == State::Idle;

    ui->btnStart->setEnabled(idle);
    ui->btnPause->setEnabled(state == State::Running);
    ui->btnResume->setEnabled(state == State::Paused);
    ui->btnStop->setEnabled(!idle);
    ui->btnClearList->setEnabled(idle);

    const QList<QWidget *> settingsWidgets = {
        ui->lineEditMask, ui->checkDeleteInput,
        ui->lineEditOutputDir, ui->btnBrowseOutput,
        ui->lineEditInputDir, ui->btnBrowseInput,
        ui->comboConflict, ui->radioOnce, ui->radioTimer,
        ui->spinInterval, ui->comboIntervalUnit,
        ui->lineEditHexKey, ui->btnValidateKey
    };
    for (QWidget *w : settingsWidgets)
        w->setEnabled(idle);

    switch (state) {
    case State::Idle:
        m_statusLabel->setText(tr("Статус: <span style=\"color:#9c8291;\">Ожидание</span>"));
        break;
    case State::Running:
        m_statusLabel->setText(tr("Статус: <span style=\"color:#35ab63;\">Работает</span>"));
        break;
    case State::Paused:
        m_statusLabel->setText(tr("Статус: <span style=\"color:#cf7f2f;\">Пауза</span>"));
        break;
    }
}

void MainWindow::resetSession()
{
    ui->tableFiles->setRowCount(0);
    m_files.clear();
    m_queue.clear();
    m_knownPaths.clear();
    m_nextId = 1;
    m_currentId = -1;
    m_doneCount = 0;
    m_currentFraction = 0.0;
    m_firstScan = true;
    ui->progressTotal->setValue(0);
    updateCounters();
    resetDetails();
}

void MainWindow::resetDetails()
{
    const QString dash = QStringLiteral("—");
    ui->labelDetName->setText(dash);
    ui->labelDetPath->setText(dash);
    ui->labelDetPath->setToolTip(QString());
    ui->labelDetSize->setText(dash);
    ui->labelDetProcessed->setText(dash);
    ui->labelDetSpeed->setText(dash);
    ui->labelDetElapsed->setText(dash);
    ui->labelDetRemaining->setText(dash);
    ui->labelDetStatus->setText(dash);
    ui->progressCurrent->setValue(0);
}

bool MainWindow::parseKey(quint64 *keyOut, QString *errorOut) const
{
    const QString text = ui->lineEditHexKey->text().trimmed();
    static const QRegularExpression re(QStringLiteral("^[0-9A-Fa-f]{16}$"));
    if (!re.match(text).hasMatch()) {
        if (errorOut)
            *errorOut = tr("Значение XOR должно содержать ровно 16 шестнадцатеричных символов (8 байт),\nнапример: 1234567890ABCDEF");
        return false;
    }
    bool ok = false;
    *keyOut = text.toULongLong(&ok, 16);
    if (!ok && errorOut)
        *errorOut = tr("Не удалось разобрать значение XOR.");
    return ok;
}

void MainWindow::appendLog(int level, const QString &text)
{
    LogEntry entry;
    entry.time = QTime::currentTime().toString(QStringLiteral("hh:mm:ss"));
    entry.level = level;
    entry.text = text;
    m_logEntries.append(entry);
    ui->logView->append(renderLogEntry(entry));
}

QString MainWindow::renderLogEntry(const LogEntry &entry) const
{
    QString name;
    QString color;
    switch (entry.level) {
    case LogWarn:
        name = QStringLiteral("WARN");
        color = m_darkTheme ? QStringLiteral("#e09a4a") : QStringLiteral("#c77b2e");
        break;
    case LogError:
        name = QStringLiteral("ERROR");
        color = m_darkTheme ? QStringLiteral("#e2604e") : QStringLiteral("#c0392b");
        break;
    default:
        name = QStringLiteral("INFO");
        color = m_darkTheme ? QStringLiteral("#3fbf72") : QStringLiteral("#2e9e5b");
        break;
    }
    return QStringLiteral(
        "<span style=\"color:%1;\">%2</span> "
        "<span style=\"color:%3;font-weight:600;\">[%4]</span> "
        "<span style=\"color:%5;\">%6</span>")
        .arg(m_logTimeColor, entry.time, color, name,
             m_logTextColor, entry.text.toHtmlEscaped());
}

void MainWindow::rebuildLog()
{
    ui->logView->clear();
    for (const LogEntry &entry : std::as_const(m_logEntries))
        ui->logView->append(renderLogEntry(entry));
}

void MainWindow::logInfo(const QString &text)
{
    appendLog(LogInfo, text);
}

void MainWindow::logWarn(const QString &text)
{
    appendLog(LogWarn, text);
}

void MainWindow::logError(const QString &text)
{
    appendLog(LogError, text);
}

QString MainWindow::formatSize(qint64 bytes)
{
    const double b = double(bytes);
    if (bytes >= (1LL << 30))
        return QString::number(b / double(1LL << 30), 'f', 2) + QStringLiteral(" GB");
    if (bytes >= (1LL << 20))
        return QString::number(b / double(1LL << 20), 'f', 2) + QStringLiteral(" MB");
    if (bytes >= (1LL << 10))
        return QString::number(b / double(1LL << 10), 'f', 2) + QStringLiteral(" KB");
    return QString::number(bytes) + tr(" байт");
}

QString MainWindow::formatSizeFull(qint64 bytes)
{
    return tr("%1 (%2 байт)")
        .arg(formatSize(bytes), QLocale::system().toString(qlonglong(bytes)));
}

QString MainWindow::formatDuration(qint64 ms)
{
    const qint64 secs = ms / 1000;
    return QStringLiteral("%1:%2:%3")
        .arg(secs / 3600, 2, 10, QLatin1Char('0'))
        .arg((secs % 3600) / 60, 2, 10, QLatin1Char('0'))
        .arg(secs % 60, 2, 10, QLatin1Char('0'));
}

QString MainWindow::formatSpeed(double bytesPerSecond)
{
    if (bytesPerSecond <= 0.0)
        return QStringLiteral("—");
    const double mb = bytesPerSecond / double(1LL << 20);
    if (mb >= 1.0)
        return QString::number(mb, 'f', mb >= 100.0 ? 0 : 1) + QStringLiteral(" MB/s");
    return QString::number(bytesPerSecond / double(1LL << 10), 'f', 0) + QStringLiteral(" KB/s");
}
