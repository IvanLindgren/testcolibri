#ifndef FILEPROCESSOR_H
#define FILEPROCESSOR_H

#include <QAtomicInteger>
#include <QMutex>
#include <QObject>
#include <QWaitCondition>

//описание класса FileProcessor, который выполняет обработку файлов в отдельном потоке!! вот умница!!
class FileProcessor : public QObject
{
    Q_OBJECT

public:
    enum Result {
        Success = 0,
        Cancelled,
        Error
    };

    explicit FileProcessor(QObject *parent = nullptr);

    
    void requestPause();
    void requestResume();
    void requestStop();
    void resetControlFlags();

public slots:
    void processFile(int id, const QString &inputPath, const QString &outputDir,
                     quint64 key, bool deleteInput, bool overwriteExisting);

signals:
    void fileStarted(int id, const QString &outputPath);
    void progressChanged(int id, qint64 processedBytes, qint64 totalBytes,
                         double bytesPerSecond, qint64 elapsedMs, qint64 remainingMs);
    void filePaused(int id);
    void fileResumed(int id);
    void fileFinished(int id, int result, const QString &message);

private:
    static QString resolveOutputPath(const QString &inputPath, const QString &outputDir,
                                     bool overwrite);

    QAtomicInteger<int> m_pauseRequested{0};
    QAtomicInteger<int> m_stopRequested{0};
    QMutex m_mutex;
    QWaitCondition m_pauseCondition;
};

#endif
