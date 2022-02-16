/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef VIEWDRAWHELPER_H
#define VIEWDRAWHELPER_H

#include "dfmplugin_workspace_global.h"

#include <QObject>
#include <QPixmap>
#include <QModelIndexList>
#include <QStyleOptionViewItem>

namespace GlobalPrivate {
const int kDragIconMaxCount = 99;
const int kDragIconSize = 128;
const int kDragIconOutline = 30;
const int kDragIconMax = 4;
const qreal kDragIconRotate = 10.0;
const qreal kDragIconOpacity = 0.1;
}

DPWORKSPACE_BEGIN_NAMESPACE

class FileView;
class FileViewModel;
class ViewDrawHelper : public QObject
{
    Q_OBJECT
public:
    explicit ViewDrawHelper(FileView *parent);

    QPixmap renderDragPixmap(QModelIndexList indexes) const;

private:
    FileViewModel *viewModel() const;
    void drawDragIcons(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QModelIndexList &indexes, const QModelIndex &topIndex) const;
    void drawDragCount(QPainter *painter, const QModelIndex &topIndex, const QStyleOptionViewItem &option, int count) const;

    FileView *view { nullptr };
};

DPWORKSPACE_END_NAMESPACE

#endif   // VIEWDRAWHELPER_H