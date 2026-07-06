#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QElapsedTimer>
#include <QHash>
#include <QMainWindow>
#include <QSet>
#include <QThread>
#include <QTimer>
#include <QVector>

class FileProcessor;
class SparkleOverlay;
class QCloseEvent;
class QLabel;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

signals:
    // соединён с FileProcessor::processFile через очередь событий рабочего потока.
    void startFileRequested(int id, const QString &inputPath, const QString &outputDir,
                            quint64 key, bool deleteInput, bool overwriteExisting);

private slots:
    void onStartClicked();
    void onPauseClicked();
    void onResumeClicked();
    void onStopClicked();
    void onValidateKeyClicked();
    void onBrowseInput();
    void onBrowseOutput();
    void onClearLog();
    void onClearList();
    void onThemeToggle();
    void onPollTimer();
    void onUptimeTick();

    void onFileStarted(int id, const QString &outputPath);
    void onFileProgress(int id, qint64 processed, qint64 total,
                        double bytesPerSecond, qint64 elapsedMs, qint64 remainingMs);
    void onFilePaused(int id);
    void onFileResumed(int id);
    void onFileFinished(int id, int result, const QString &message);

private:
    enum class State { Idle, Running, Paused };
    enum LogLevel { LogInfo, LogWarn, LogError };

    struct FileEntry {
        QString path;
        qint64 size = 0;
        int row = -1;
        bool done = false;
    };

    struct LogEntry {
        QString time;
        int level = LogInfo;
        QString text;
    };

    void setState(State state);
    void shutdownWorker();
    void resetSession();
    void resetDetails();
    void scanInputDirectory();
    void dispatchNext();
    void finishSessionIfDone();
    void addFileRow(int id);
    void setRowStatus(int id, const QString &text, const QColor &color);
    void setProgressForRow(int row, int percent);
    void updateCounters();
    void updateOverallProgress();
    bool parseKey(quint64 *keyOut, QString *errorOut) const;

    void applyTheme(bool dark);
    void appendLog(int level, const QString &text);
    QString renderLogEntry(const LogEntry &entry) const;
    void rebuildLog();
    void logInfo(const QString &text);
    void logWarn(const QString &text);
    void logError(const QString &text);

    static QString formatSize(qint64 bytes);
    static QString formatSizeFull(qint64 bytes);
    static QString formatDuration(qint64 ms);
    static QString formatSpeed(double bytesPerSecond);

    Ui::MainWindow *ui;

    QThread m_workerThread;
    FileProcessor *m_processor = nullptr;
    SparkleOverlay *m_sparkles = nullptr;

    bool m_darkTheme = false;
    QString m_logTimeColor;
    QString m_logTextColor;
    QVector<LogEntry> m_logEntries;

    QTimer m_pollTimer;
    QTimer m_uptimeTimer;
    QElapsedTimer m_uptime;

    QLabel *m_statusLabel = nullptr;
    QLabel *m_modeLabel = nullptr;
    QLabel *m_uptimeLabel = nullptr;

    State m_state = State::Idle;
    QHash<int, FileEntry> m_files;
    QVector<int> m_queue;
    QSet<QString> m_knownPaths;
    int m_nextId = 1;
    int m_currentId = -1;
    int m_doneCount = 0;
    double m_currentFraction = 0.0;
    bool m_firstScan = true;

    quint64 m_key = 0;
    QString m_inputDir;
    QString m_outputDir;
    QStringList m_masks;
    bool m_deleteInput = false;
    bool m_overwrite = false;
    bool m_timerMode = false;
};

#endif
