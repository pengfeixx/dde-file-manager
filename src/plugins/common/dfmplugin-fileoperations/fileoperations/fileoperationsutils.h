/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef FILEOPERATIONSUTILS_H
#define FILEOPERATIONSUTILS_H

#include "dfmplugin_fileoperations_global.h"
#include "dfm-base/interfaces/abstractjobhandler.h"
#include "services/common/dialog/dialogservice.h"
#include "services/common/fileoperations/fileoperationsservice.h"

#include <QObject>
#include <QPointer>

class QTimer;

DPFILEOPERATIONS_BEGIN_NAMESPACE

class FileOperationsUtils : public QObject
{
    Q_OBJECT
public:
    explicit FileOperationsUtils(QObject *parent = nullptr);
    ~FileOperationsUtils() override = default;

    JobHandlePointer copy(const QList<QUrl> &sources, const QUrl &target,
                          const dfmbase::AbstractJobHandler::JobFlags &flags = dfmbase::AbstractJobHandler::JobFlag::kNoHint);
    JobHandlePointer moveToTrash(const QList<QUrl> &sources,
                                 const dfmbase::AbstractJobHandler::JobFlags &flags = dfmbase::AbstractJobHandler::JobFlag::kNoHint);
    JobHandlePointer restoreFromTrash(const QList<QUrl> &sources,
                                      const dfmbase::AbstractJobHandler::JobFlags &flags = dfmbase::AbstractJobHandler::JobFlag::kNoHint);
    JobHandlePointer deletes(const QList<QUrl> &sources,
                             const dfmbase::AbstractJobHandler::JobFlags &flags = dfmbase::AbstractJobHandler::JobFlag::kNoHint);
    JobHandlePointer cut(const QList<QUrl> &sources, const QUrl &target,
                         const dfmbase::AbstractJobHandler::JobFlags &flags = dfmbase::AbstractJobHandler::JobFlag::kNoHint);

private:
    bool getOperationsAndDialogService();
    void initArguments(const JobHandlePointer &handler);
private slots:
    void onHandleAddTask();
    void onHandleAddTaskWithArgs(const JobInfoPointer info);
    void onHandleTaskFinished(const JobInfoPointer info);

private:
    QMap<JobHandlePointer, QSharedPointer<QTimer>> copyMoveTask;
    QSharedPointer<QMutex> copyMoveTaskMutex { nullptr };
    QSharedPointer<QMutex> getOperationsAndDialogServiceMutex { nullptr };
    QPointer<DSC_NAMESPACE::FileOperationsService> operationsService { nullptr };
    QPointer<DSC_NAMESPACE::DialogService> dialogService { nullptr };
};

DPFILEOPERATIONS_END_NAMESPACE

#endif   // FILEOPERATIONSUTILS_H