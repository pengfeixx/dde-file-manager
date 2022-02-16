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
#ifndef OPERATIONSSTACKMANAGERDBUS_H
#define OPERATIONSSTACKMANAGERDBUS_H

#include <QDBusVariant>
#include <QVariantMap>
#include <QStack>
#include <QMutex>
#include <QObject>

class OperationsStackManagerDbus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.filemanager.service.OperationsStackManager")

public:
    explicit OperationsStackManagerDbus(QObject *parent = nullptr);
    ~OperationsStackManagerDbus();

public slots:
    void SaveOperations(const QVariantMap &values);
    void CleanOperations();
    QVariantMap RevocationOperations();

private:
    QStack<QVariantMap> fileOperations;
    QMutex lock;
};

#endif   // OPERATIONSSTACKMANAGERDBUS_H