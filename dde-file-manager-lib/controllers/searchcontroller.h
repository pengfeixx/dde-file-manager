#ifndef SEARCHCONTROLLER_H
#define SEARCHCONTROLLER_H

#include "dabstractfilecontroller.h"

#include <QSet>
#include <QPair>

class SearchFileWatcher;
class SearchController : public DAbstractFileController
{
    Q_OBJECT

public:
    explicit SearchController(QObject *parent = 0);

    const DAbstractFileInfoPointer createFileInfo(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool openFileLocation(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;

    bool openFile(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool openFileByApp(const DUrl &fileUrl, const QString& app, bool &accepted) const Q_DECL_OVERRIDE;
    bool copyFilesToClipboard(const DUrlList &urlList, bool &accepted) const Q_DECL_OVERRIDE;
    DUrlList moveToTrash(const DFMEvent &event, bool &accepted) const Q_DECL_OVERRIDE;
    bool cutFilesToClipboard(const DUrlList &urlList, bool &accepted) const Q_DECL_OVERRIDE;
    bool deleteFiles(const DFMEvent &event, bool &accepted) const Q_DECL_OVERRIDE;
    bool renameFile(const DUrl &oldUrl, const DUrl &newUrl, bool &accepted) const Q_DECL_OVERRIDE;

    bool compressFiles(const DUrlList &urlList, bool &accepted) const Q_DECL_OVERRIDE;
    bool decompressFile(const DUrlList &fileUrlList, bool &accepted) const Q_DECL_OVERRIDE;

    bool createSymlink(const DUrl &fileUrl, const DUrl &linkToUrl, bool &accepted) const Q_DECL_OVERRIDE;

    bool unShareFolder(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool openInTerminal(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;

    const DDirIteratorPointer createDirIterator(const DUrl &fileUrl, const QStringList &nameFilters,
                                                QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                bool &accepted) const Q_DECL_OVERRIDE;

    DAbstractFileWatcher *createFileWatcher(const DUrl &fileUrl, QObject *parent, bool &accepted) const Q_DECL_OVERRIDE;

private:
    static DUrl realUrl(const DUrl &searchUrl);
    static DUrlList realUrlList(const DUrlList &searchUrls);

    friend class SearchDiriterator;
    friend class SearchFileWatcher;
};

#endif // SEARCHCONTROLLER_H
