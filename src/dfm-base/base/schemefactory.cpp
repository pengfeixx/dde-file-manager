/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
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
#include "schemefactory.h"

DFMBASE_BEGIN_NAMESPACE

InfoFactory &InfoFactory::instance()
{
    if (!ins) {
        ins = new InfoFactory();
        static GC<InfoFactory> infoFactory_gc(ins);
    }
    return *ins;
}

WacherFactory &WacherFactory::instance()
{
    if (!ins) {
        ins = new WacherFactory();
        static GC<WacherFactory> watcherFactory_gc(ins);
    }
    return *ins;
}

FileDeviceFactory &FileDeviceFactory::instance()
{
    if (!ins) {
        ins = new FileDeviceFactory();
        static GC<FileDeviceFactory> fileDeviceFactory_gc(ins);
    }
    return *ins;
}

DirIteratorFactory &DirIteratorFactory::instance()
{
    if (!ins) {
        ins = new DirIteratorFactory();
        static GC<DirIteratorFactory> dirIteratorFactory(ins);
    }
    return *ins;
}

DFMBASE_END_NAMESPACE