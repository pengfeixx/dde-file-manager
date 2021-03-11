/*
 * Copyright (C) 2016 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
*/

#pragma once

#include "dfmsidebariteminterface.h"

#define SIDEBAR_ID_DEFAULT "__default"

DFM_BEGIN_NAMESPACE

class DFMSideBarDefaultItemHandler : public DFMSideBarItemInterface
{
public:
    static DFMSideBarItem * createItem(const QString &pathKey);

    explicit DFMSideBarDefaultItemHandler(QObject *parent = nullptr);

    void cdAction(const DFMSideBar *sidebar, const DFMSideBarItem* item) override;
    QMenu * contextMenu(const DFMSideBar *sidebar, const DFMSideBarItem* item) override;
};

DFM_END_NAMESPACE