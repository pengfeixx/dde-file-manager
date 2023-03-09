// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filestatisticsjob.h"
#include "interfaces/abstractfileinfo.h"
#include "base/schemefactory.h"
#include "interfaces/abstractdiriterator.h"

#include "dfm-base/utils/universalutils.h"

#include <dfm-io/dfmio_utils.h>

#include <QMutex>
#include <QQueue>
#include <QTimer>
#include <QWaitCondition>
#include <QStorageInfo>
#include <QElapsedTimer>
#include <QDebug>

#include <fts.h>
#include <sys/stat.h>
#include <dirent.h>

namespace dfmbase {

static constexpr uint16_t kSizeChangeinterval { 200 };

class FileStatisticsJobPrivate : public QObject
{
public:
    explicit FileStatisticsJobPrivate(FileStatisticsJob *qq);
    ~FileStatisticsJobPrivate();

    void setState(FileStatisticsJob::State s);

    bool jobWait();
    bool stateCheck();

    void processFile(const QUrl &url, const bool followLink, QQueue<QUrl> &directoryQueue);
    void emitSizeChanged();
    int countFileCount(const char *name, bool isloop = false);

    FileStatisticsJob *q;
    QTimer *notifyDataTimer;

    QAtomicInt state = FileStatisticsJob::kStoppedState;
    FileStatisticsJob::FileHints fileHints;

    QList<QUrl> sourceUrlList;
    QWaitCondition waitCondition;
    QElapsedTimer elapsedTimer;

    QAtomicInteger<qint64> totalSize = { 0 };
    QAtomicInteger<qint64> totalProgressSize { 0 };
    QAtomicInt filesCount { 0 };
    QAtomicInt directoryCount { 0 };
    SizeInfoPointer sizeInfo { nullptr };
    QList<QUrl> fileStatistics;
};

FileStatisticsJobPrivate::FileStatisticsJobPrivate(FileStatisticsJob *qq)
    : QObject(nullptr), q(qq), notifyDataTimer(nullptr)
{
    sizeInfo.reset(new FileUtils::FilesSizeInfo());
    sizeInfo->dirSize = FileUtils::getMemoryPageSize();
}

FileStatisticsJobPrivate::~FileStatisticsJobPrivate()
{
    if (notifyDataTimer) {
        notifyDataTimer->stop();
        notifyDataTimer->deleteLater();
    }
}

void FileStatisticsJobPrivate::setState(FileStatisticsJob::State s)
{
    if (s == state) {
        return;
    }

    state = s;

    if (notifyDataTimer->thread() && notifyDataTimer->thread()->loopLevel() <= 0) {
        qWarning() << "The thread of notify data timer no event loop" << notifyDataTimer->thread();
    }

    if (s == FileStatisticsJob::kRunningState) {
        QMetaObject::invokeMethod(notifyDataTimer, "start", Q_ARG(int, 500));
        elapsedTimer.start();
    } else {
        QMetaObject::invokeMethod(notifyDataTimer, "stop");

        if (s == FileStatisticsJob::kStoppedState) {
            Q_EMIT q->dataNotify(totalSize, filesCount, directoryCount);
            Q_EMIT q->sizeChanged(totalSize);
        }

        qDebug() << "statistic finished(may stop), result: " << totalSize << filesCount << directoryCount;
    }

    Q_EMIT q->stateChanged(s);
}

bool FileStatisticsJobPrivate::jobWait()
{
    QMutex lock;

    lock.lock();
    waitCondition.wait(&lock);
    lock.unlock();

    return state == FileStatisticsJob::kRunningState;
}

bool FileStatisticsJobPrivate::stateCheck()
{
    if (state == FileStatisticsJob::kRunningState) {
        return true;
    }

    if (state == FileStatisticsJob::kPausedState) {
        if (!jobWait()) {
            return false;
        }
    } else if (state == FileStatisticsJob::kStoppedState) {
        return false;
    }

    return true;
}

void FileStatisticsJobPrivate::processFile(const QUrl &url, const bool followLink, QQueue<QUrl> &directoryQueue)
{
    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);

    if (!info) {
        qDebug() << "Url not yet supported: " << url;
        return;
    }

    qint64 size = 0;

    if (info->isAttributes(OptInfoType::kIsFile)) {
        do {
            auto isSyslink = info->isAttributes(OptInfoType::kIsSymLink);
            if (isSyslink) {
                const auto &symLinkTargetUrl = QUrl::fromLocalFile(info->pathOf(PathInfoType::kSymLinkTarget));
                if (sizeInfo->allFiles.contains(symLinkTargetUrl) || fileStatistics.contains(symLinkTargetUrl)) {
                    return;
                }
                fileStatistics << symLinkTargetUrl;
            }

            // ###(zccrs): skip the file,os file
            if (UniversalUtils::urlEquals(info->urlOf(UrlInfoType::kUrl), QUrl::fromLocalFile("/proc/kcore"))
                || UniversalUtils::urlEquals(info->urlOf(UrlInfoType::kUrl), QUrl::fromLocalFile("/dev/core"))) {
                break;
            }
            //skip os file Shortcut
            if (info->isAttributes(OptInfoType::kIsSymLink)
                && (info->pathOf(PathInfoType::kSymLinkTarget)
                            == QStringLiteral("/proc/kcore")
                    || info->pathOf(PathInfoType::kSymLinkTarget) == QStringLiteral("/dev/core"))) {
                break;
            }

            const AbstractFileInfo::FileType type = info->fileType();

            if (type == AbstractFileInfo::FileType::kCharDevice && !fileHints.testFlag(FileStatisticsJob::kDontSkipCharDeviceFile)) {
                break;
            }

            if (type == AbstractFileInfo::FileType::kBlockDevice && !fileHints.testFlag(FileStatisticsJob::kDontSkipBlockDeviceFile)) {
                break;
            }

            if (type == AbstractFileInfo::FileType::kFIFOFile && !fileHints.testFlag(FileStatisticsJob::kDontSkipFIFOFile)) {
                break;
            }

            if (type == AbstractFileInfo::FileType::kSocketFile && !fileHints.testFlag(FileStatisticsJob::kDontSkipSocketFile)) {
                break;
            }
            if (type == AbstractFileInfo::FileType::kUnknown) {
                break;
            }

            size = info->size();
            if (size > 0) {
                totalSize += size;
                emitSizeChanged();
            }
            // fix bug 30548 ,以为有些文件大小为0,文件夹为空，size也为零，重新计算显示大小
            // fix bug 202007010033【文件管理器】【5.1.2.10-1】【sp2】复制软连接的文件，进度条显示1%
            // 判断文件是否是链接文件
            totalProgressSize += (size <= 0 || isSyslink) ? FileUtils::getMemoryPageSize() : size;
        } while (false);

        ++filesCount;
    } else {
        // fix bug 30548 ,以为有些文件大小为0,文件夹为空，size也为零，重新计算显示大小
        totalProgressSize += FileUtils::getMemoryPageSize();
        if (info->isAttributes(OptInfoType::kIsSymLink)) {
            if (!followLink) {
                ++directoryCount;
                return;
            }

            do {
                info = InfoFactory::create<AbstractFileInfo>(QUrl::fromLocalFile(info->pathOf(PathInfoType::kSymLinkTarget)));
            } while (info && info->isAttributes(OptInfoType::kIsSymLink));

            if (!info) {
                ++directoryCount;
                return;
            }

            const auto &symLinkTargetUrl = QUrl::fromLocalFile(info->pathOf(PathInfoType::kSymLinkTarget));
            if (sizeInfo->allFiles.contains(symLinkTargetUrl) || fileStatistics.contains(symLinkTargetUrl)) {
                return;
            }
            fileStatistics << symLinkTargetUrl;
        }

        ++directoryCount;

        if (!(fileHints & (FileStatisticsJob::kDontSkipAVFSDStorage | FileStatisticsJob::kDontSkipPROCStorage)) && dfmbase::FileUtils::isLocalFile(info->urlOf(UrlInfoType::kUrl))) {
            do {
                QStorageInfo si(info->urlOf(UrlInfoType::kUrl).toLocalFile());

                if (si.rootPath() == info->urlOf(UrlInfoType::kUrl).toLocalFile()) {
                    if (!fileHints.testFlag(FileStatisticsJob::kDontSkipPROCStorage)
                        && si.device() == "proc") {
                        break;
                    }

                    if (!fileHints.testFlag(FileStatisticsJob::kDontSkipAVFSDStorage)
                        && si.device() == "avfsd") {
                        break;
                    }
                }

                if (!fileHints.testFlag(FileStatisticsJob::kSingleDepth))
                    directoryQueue << url;
            } while (false);
        } else if (!fileHints.testFlag(FileStatisticsJob::kSingleDepth)) {
            directoryQueue << url;
        }
    }
}

void FileStatisticsJobPrivate::emitSizeChanged()
{
    if (elapsedTimer.elapsed() > kSizeChangeinterval) {
        Q_EMIT q->sizeChanged(totalSize);
        elapsedTimer.restart();
    }
}

int FileStatisticsJobPrivate::countFileCount(const char *name, bool isloop)
{
    DIR *dir;
    struct dirent *entry;
    int fileCount = 0;

    if (!(dir = opendir(name)))
        return fileCount;

    while ((entry = readdir(dir))) {
        if (!stateCheck())
            return fileCount;
        //文件路径加名称最长支持为4k
        char path[FILENAME_MAX + 1];
        int len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);
        path[len] = 0;
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        if (entry->d_type == DT_DIR) {
            if (isloop) {
                fileCount += countFileCount(path, isloop);
            }
            else {
                fileCount++;
            }
        }
        else {
            fileCount++;
        }
    }

    closedir(dir);
    return fileCount;
}

FileStatisticsJob::FileStatisticsJob(QObject *parent)
    : QThread(parent), d(new FileStatisticsJobPrivate(this))
{
    d->notifyDataTimer = new QTimer(this);

    connect(d->notifyDataTimer, &QTimer::timeout, this, [this] {
        Q_EMIT dataNotify(d->totalSize, d->filesCount, d->directoryCount);
    },
            Qt::DirectConnection);
}

FileStatisticsJob::~FileStatisticsJob()
{
    stop();
    wait();
}

FileStatisticsJob::State FileStatisticsJob::state() const
{
    return static_cast<FileStatisticsJob::State>(d->state.load());
}

FileStatisticsJob::FileHints FileStatisticsJob::fileHints() const
{
    return d->fileHints;
}

qint64 FileStatisticsJob::totalSize() const
{
    return d->totalSize.load();
}
// fix bug 30548 ,以为有些文件大小为0,文件夹为空，size也为零，重新计算显示大小
qint64 FileStatisticsJob::totalProgressSize() const
{
    return d->totalProgressSize;
}

int FileStatisticsJob::filesCount() const
{
    return d->filesCount.load();
}

int FileStatisticsJob::directorysCount(bool includeSelf) const
{
    if (includeSelf) {
        return d->directoryCount.load();
    } else {
        return qMax(d->directoryCount.load() - 1, 0);
    }
}

SizeInfoPointer FileStatisticsJob::getFileSizeInfo()
{
    return d->sizeInfo;
}

void FileStatisticsJob::start(const QList<QUrl> &sourceUrls)
{
    if (isRunning()) {
        qDebug() << "current thread is running... reject to start.";
        return;
    }
    d->sourceUrlList = sourceUrls;

    if (d->sourceUrlList.count() <= 0) {
        return;
    }

    QThread::start();
}

void FileStatisticsJob::stop()
{
    if (d->state == kStoppedState) {
        return;
    }

    d->setState(kStoppedState);
    d->waitCondition.wakeAll();
}

void FileStatisticsJob::togglePause()
{

    if (d->state == kStoppedState) {
        return;
    }

    if (d->state == kPausedState) {
        d->setState(kRunningState);
        d->waitCondition.wakeAll();
    } else {
        d->setState(kPausedState);
    }
}

void FileStatisticsJob::setFileHints(FileHints fileHints)
{
    Q_ASSERT(d->state != kRunningState);

    d->fileHints = fileHints;
}

void FileStatisticsJob::run()
{
    d->setState(kRunningState);
    d->totalSize = 0;
    d->filesCount = 0;
    d->directoryCount = 0;

    statistcsOtherFileSystem();
}

void FileStatisticsJob::setSizeInfo()
{
    d->sizeInfo->fileCount = static_cast<quint32>(d->filesCount);
    d->sizeInfo->totalSize = d->totalProgressSize;
    d->sizeInfo->dirSize = d->sizeInfo->dirSize == 0 ? FileUtils::getMemoryPageSize() : d->sizeInfo->dirSize;
}

void FileStatisticsJob::statistcsOtherFileSystem()
{
    Q_EMIT dataNotify(0, 0, 0);

    const bool followLink = !d->fileHints.testFlag(kNoFollowSymlink);

    QQueue<QUrl> directory_queue;
    int fileCount = 0;
    if (d->fileHints.testFlag(kExcludeSourceFile)) {
        for (const QUrl &url : d->sourceUrlList) {
            if (!d->stateCheck()) {
                d->setState(kStoppedState);
                setSizeInfo();
                return;
            }
            // The files counted are not counted
            if (d->sizeInfo->allFiles.contains(url))
                continue;

            d->sizeInfo->allFiles << url;
            AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);

            if (!info) {
                qDebug() << "Url not yet supported: " << url;
                continue;
            }

            if (info->isAttributes(OptInfoType::kIsDir) && d->fileHints.testFlag(kSingleDepth)) {
                fileCount += d->countFileCount(info->pathOf(PathInfoType::kPath).toStdString().c_str());
            } else {
                fileCount++;
            }

            if (info->isAttributes(OptInfoType::kIsSymLink)) {
                if (!followLink) {
                    continue;
                }

                const auto &symLinkTargetUrl = QUrl::fromLocalFile(info->pathOf(PathInfoType::kSymLinkTarget));
                // The files counted are not counted
                if (d->fileStatistics.contains(symLinkTargetUrl) || d->sizeInfo->allFiles.contains(symLinkTargetUrl))
                    continue;

                info = InfoFactory::create<AbstractFileInfo>(symLinkTargetUrl);

                if (info->isAttributes(OptInfoType::kIsSymLink)) {
                    continue;
                }

                d->fileStatistics << symLinkTargetUrl;
            }

            if (info->isAttributes(OptInfoType::kIsDir)) {
                if (d->sizeInfo->dirSize == 0) {
                    struct stat statInfo;

                    if (0 == stat(info->urlOf(UrlInfoType::kUrl).path().toStdString().data(), &statInfo))
                        d->sizeInfo->dirSize = statInfo.st_size == 0 ? FileUtils::getMemoryPageSize() : static_cast<quint16>(statInfo.st_size);
                }
                directory_queue << url;
            }
        }
    } else {
        for (const QUrl &url : d->sourceUrlList) {
            // 选择的列表中包含avfsd/proc挂载路径时禁用过滤
            FileHints save_file_hints = d->fileHints;
            d->fileHints = d->fileHints | kDontSkipAVFSDStorage | kDontSkipPROCStorage;
            d->processFile(url, followLink, directory_queue);
            d->sizeInfo->allFiles << url;
            d->fileHints = save_file_hints;

            if (!d->stateCheck()) {
                d->setState(kStoppedState);
                setSizeInfo();
                return;
            }
        }
    }

    if (d->fileHints.testFlag(kSingleDepth)) {
        d->filesCount = fileCount;
        directory_queue.clear();
        setSizeInfo();
        return;
    }

    while (!directory_queue.isEmpty()) {
        const QUrl &directory_url = directory_queue.dequeue();
        const AbstractDirIteratorPointer &iterator = DirIteratorFactory::create<AbstractDirIterator>(directory_url, QStringList(),
                                                                                                     QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);

        if (!iterator) {
            qWarning() << "Failed on create dir iterator, for url:" << directory_url;
            continue;
        }
        while (iterator->hasNext()) {
            QUrl url = iterator->next();
            // The files counted are not counted
            if (d->sizeInfo->allFiles.contains(url))
                continue;

            d->processFile(url, followLink, directory_queue);
            d->sizeInfo->allFiles << url;

            if (!d->stateCheck()) {
                d->setState(kStoppedState);
                setSizeInfo();
                return;
            }
        }
    }
    setSizeInfo();
    d->setState(kStoppedState);
}

}