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
#include "listener.h"
#include "dfm-framework/listener/private/listener_p.h"

DPF_BEGIN_NAMESPACE

Listener::Listener(QObject *parent)
    : QObject(parent)
    , d(new ListenerPrivate(this))
{

}

Listener &Listener::instance()
{
    if (!self)
        self = new Listener();
    return *self;
}

ListenerPrivate::ListenerPrivate(Listener *parent)
    : QObject(parent)
    , q_ptr(parent)
{
    QObject::connect(this, &ListenerPrivate::pluginsInitialized,
                     q_ptr, &Listener::pluginsInitialized,
                     Qt::UniqueConnection);

    QObject::connect(this, &ListenerPrivate::pluginsStarted,
                     q_ptr, &Listener::pluginsStarted,
                     Qt::UniqueConnection);

    QObject::connect(this, &ListenerPrivate::pluginsStoped,
                     q_ptr, &Listener::pluginsStoped,
                     Qt::UniqueConnection);
}

DPF_END_NAMESPACE