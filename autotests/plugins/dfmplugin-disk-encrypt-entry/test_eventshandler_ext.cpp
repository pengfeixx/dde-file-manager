// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "events/eventshandler.h"
#include "dfmplugin_disk_encrypt_global.h"
#include "services/diskencrypt/globaltypesdefine.h"

#include <gtest/gtest.h>

#include <QApplication>
#include <QDBusInterface>
#include <QDBusAbstractInterface>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QFile>
#include <QTemporaryDir>

#include <DDBusSender>

using namespace dfmplugin_diskenc;
using namespace disk_encrypt;

class EventsHandlerExtTest : public testing::Test
{
protected:
    void SetUp() override
    {
        ins = EventsHandler::instance();
        stub.set_lamda(&QDBusAbstractInterface::isValid, [](QDBusAbstractInterface *) -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });
    }
    void TearDown() override { stub.clear(); }
    stub_ext::StubExt stub;
    EventsHandler *ins = nullptr;

    void stubAsyncCall()
    {
        using AsyncFunc = QDBusPendingCall (QDBusAbstractInterface::*)(const QString &, const QList<QVariant> &);
        stub.set_lamda(static_cast<AsyncFunc>(&QDBusAbstractInterface::asyncCallWithArgumentList),
                       [](QDBusAbstractInterface *, const QString &, const QList<QVariant> &) -> QDBusPendingCall {
                           __DBG_STUB_INVOKE__
                           return QDBusPendingCall(nullptr);
                       });
        stub.set_lamda(&QDBusAbstractInterface::isValid, [](QDBusAbstractInterface *) -> bool { return true; });
    }
};

TEST_F(EventsHandlerExtTest, SetAutoStartDFM_Enable)
{
    stubAsyncCall();
    EXPECT_NO_FATAL_FAILURE(ins->setAutoStartDFM(true));
}

TEST_F(EventsHandlerExtTest, SetAutoStartDFM_Disable)
{
    stubAsyncCall();
    EXPECT_NO_FATAL_FAILURE(ins->setAutoStartDFM(false));
}

TEST_F(EventsHandlerExtTest, OnEncryptProgress_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(ins->onEncryptProgress("/dev/sda1", "encrypting", 0.5));
}

TEST_F(EventsHandlerExtTest, OnDecryptProgress_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(ins->onDecryptProgress("/dev/sda1", "decrypting", 0.3));
}

TEST_F(EventsHandlerExtTest, OnInitEncryptFinished_NoCrash)
{
    QVariantMap info;
    info.insert("device-path", "/dev/sda1");
    EXPECT_NO_FATAL_FAILURE(ins->onInitEncryptFinished(info));
}

TEST_F(EventsHandlerExtTest, RequestReboot_NoCrash)
{
    using CallFunc = QDBusPendingCall (DDBusCaller::*)();
    stub.set_lamda(static_cast<CallFunc>(&DDBusCaller::call),
                   []() -> QDBusPendingCall {
                       __DBG_STUB_INVOKE__
                       return QDBusPendingCall(nullptr);
                   });
    EXPECT_NO_FATAL_FAILURE(ins->requestReboot());
}

TEST_F(EventsHandlerExtTest, HookEvents_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(ins->hookEvents());
}
