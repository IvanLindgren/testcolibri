#include "fileprocessor.h"

#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>

#include <array>
#include <cstring>

namespace {

constexpr qint64 kBufferSize = 8LL << 20;

/* xor буфера с 8-байтным ключом. ключ накладывается циклически по всему файлу,
поэтому позиция байта в файле (fileOffset) учитывается явно, т.е размер блока
и размер файла не обязаны быть кратны 8. */

void applyXor(char *data, qint64 len, quint64 key, qint64 fileOffset)
{
    std::array<quint8, 8> keyBytes{};
    // байты берутся в с большего конца: "1234567890ABCDEF" -> 0x12, 0x34, ...
    for (int i = 0; i < 8; ++i)
        keyBytes[i] = quint8(key >> (56 - 8 * i));

    qint64 i = 0;
    const qint64 phase = fileOffset & 7;
    const qint64 prefix = phase == 0 ? 0 : qMin(len, 8 - phase);

    for (; i < prefix; ++i)
        data[i] = char(quint8(data[i]) ^ keyBytes[(fileOffset + i) & 7]);

    quint64 keyBlock = 0;
    std::memcpy(&keyBlock, keyBytes.data(), sizeof(keyBlock));

    for (; i + qint64(sizeof(keyBlock)) <= len; i += sizeof(keyBlock)) {
        quint64 block = 0;
        std::memcpy(&block, data + i, sizeof(block));
        block ^= keyBlock;
        std::memcpy(data + i, &block, sizeof(block));
    }

    for (; i < len; ++i)
        data[i] = char(quint8(data[i]) ^ keyBytes[(fileOffset + i) & 7]);
}

}

FileProcessor::FileProcessor(QObject *parent)
    : QObject(parent)
{
}

void FileProcessor::requestPause()
{
    m_pauseRequested.storeRelease(1);
}

void FileProcessor::requestResume()
{
    m_pauseRequested.storeRelease(0);
    m_pauseCondition.wakeAll();
}

void FileProcessor::requestStop()
{
    m_stopRequested.storeRelease(1);
    // снимаем паузу, чтобы поток проснулся и корректно дошёл до конца метода.
    m_pauseRequested.storeRelease(0);
    m_pauseCondition.wakeAll();
}

void FileProcessor::resetControlFlags()
{
    m_stopRequested.storeRelease(0);
    m_pauseRequested.storeRelease(0);
}

QString FileProcessor::resolveOutputPath(const QString &inputPath, const QString &outputDir,
                                         bool overwrite)
{
    const QFileInfo inInfo(inputPath);
    const QDir outDir(outputDir);

    QString candidate = outDir.filePath(inInfo.fileName());
    if (overwrite || !QFileInfo::exists(candidate))
        return candidate;

    const QString base = inInfo.completeBaseName();
    const QString suffix = inInfo.suffix();
    for (int i = 1;; ++i) {
        const QString name = suffix.isEmpty()
                ? QStringLiteral("%1_%2").arg(base).arg(i)
                : QStringLiteral("%1_%2.%3").arg(base).arg(i).arg(suffix);
        candidate = outDir.filePath(name);
        if (!QFileInfo::exists(candidate))
            return candidate;
    }
}

void FileProcessor::processFile(int id, const QString &inputPath, const QString &outputDir,
                                quint64 key, bool deleteInput, bool overwriteExisting)
{
    QFile in(inputPath);
    if (!in.open(QIODevice::ReadOnly)) {
        emit fileFinished(id, Error,
                          tr("не удалось открыть входной файл (%1)").arg(in.errorString()));
        return;
    }

    if (!QDir().mkpath(outputDir)) {
        emit fileFinished(id, Error, tr("не удалось создать папку результатов"));
        return;
    }

    const QString outputPath = resolveOutputPath(inputPath, outputDir, overwriteExisting);

    // если вход и выход совпадают, пишем во временный файл и подменяем оригинал после успеха.
    const bool inPlace = QFileInfo(outputPath).absoluteFilePath()
                             .compare(QFileInfo(inputPath).absoluteFilePath(),
                                      Qt::CaseInsensitive) == 0;
    const QString writePath = inPlace ? outputPath + QStringLiteral(".xortmp") : outputPath;

    QFile out(writePath);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        emit fileFinished(id, Error,
                          tr("не удалось создать выходной файл (%1)").arg(out.errorString()));
        return;
    }

    emit fileStarted(id, outputPath);

    const qint64 total = in.size();
    QByteArray buffer(int(kBufferSize), Qt::Uninitialized);
    qint64 processed = 0;
    qint64 pausedMs = 0;
    bool stopped = false;
    QString errorText;

    QElapsedTimer clock;
    clock.start();
    QElapsedTimer emitTimer;
    emitTimer.start();

    const auto emitProgress = [&]() {
        const qint64 activeMs = clock.elapsed() - pausedMs;
        const double bytesPerSec = activeMs > 0 ? processed * 1000.0 / activeMs : 0.0;
        const qint64 remainingMs = bytesPerSec > 0
                ? qint64((total - processed) / bytesPerSec * 1000.0)
                : 0;
        emit progressChanged(id, processed, total, bytesPerSec, activeMs, remainingMs);
    };

    emitProgress();

    while (processed < total) {
        if (m_stopRequested.loadAcquire()) {
            stopped = true;
            break;
        }

        if (m_pauseRequested.loadAcquire()) {
            emit filePaused(id);
            QElapsedTimer pauseClock;
            pauseClock.start();
            m_mutex.lock();
            while (m_pauseRequested.loadAcquire() && !m_stopRequested.loadAcquire())
                m_pauseCondition.wait(&m_mutex, 250);
            m_mutex.unlock();
            pausedMs += pauseClock.elapsed();
            if (m_stopRequested.loadAcquire()) {
                stopped = true;
                break;
            }
            // позиция в файле сохранена через processed/file.pos(), поэтому продолжаем с того же места.
            emit fileResumed(id);
            continue;
        }

        const qint64 bytesRead = in.read(buffer.data(), kBufferSize);
        if (bytesRead < 0) {
            errorText = tr("ошибка чтения (%1)").arg(in.errorString());
            break;
        }
        if (bytesRead == 0)
            break;

        applyXor(buffer.data(), bytesRead, key, processed);

        if (out.write(buffer.constData(), bytesRead) != bytesRead) {
            errorText = tr("ошибка записи (%1)").arg(out.errorString());
            break;
        }
        processed += bytesRead;

        if (emitTimer.elapsed() >= 100 || processed == total) {
            emitProgress();
            emitTimer.restart();
        }
    }

    in.close();
    out.close();

    if (stopped || !errorText.isEmpty()) {
        out.remove();
        emit fileFinished(id, stopped ? Cancelled : Error,
                          stopped ? tr("обработка остановлена") : errorText);
        return;
    }

    if (inPlace) {
        if (!QFile::remove(inputPath) || !QFile::rename(writePath, outputPath)) {
            QFile::remove(writePath);
            emit fileFinished(id, Error, tr("не удалось заменить исходный файл"));
            return;
        }
    } else if (deleteInput) {
        if (!QFile::remove(inputPath)) {
            emit fileFinished(id, Success,
                              tr("готово, но не удалось удалить входной файл"));
            return;
        }
    }

    emit fileFinished(id, Success, QString());
}
