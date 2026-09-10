// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "gui/encryptparamsinputdialog.h"
#include "dfmplugin_disk_encrypt_global.h"
#include "services/diskencrypt/globaltypesdefine.h"

#include <gtest/gtest.h>
#include <QApplication>
#include <QVariantMap>

#include <dconfig.h>

using namespace dfmplugin_diskenc;
using namespace disk_encrypt;

class EncryptParamsInputDialogTest : public testing::Test
{
protected:
    void TearDown() override { stub.clear(); }
    stub_ext::StubExt stub;

    void stubDConfig(const QVariant &returnValue)
    {
        auto createFunc = static_cast<Dtk::Core::DConfig *(*)(const QString &, const QString &, const QString &, QObject *)>(&Dtk::Core::DConfig::create);
        auto fakeCfg = new Dtk::Core::DConfig("", QString(), nullptr);
        stub.set_lamda(createFunc, [fakeCfg](const QString &, const QString &, const QString &, QObject *) -> Dtk::Core::DConfig * {
            __DBG_STUB_INVOKE__
            return fakeCfg;
        });
        stub.set_lamda(&Dtk::Core::DConfig::value, [returnValue](Dtk::Core::DConfig *, const QString &, const QVariant &) -> QVariant {
            __DBG_STUB_INVOKE__
            return returnValue;
        });
    }
};

TEST_F(EncryptParamsInputDialogTest, Constructor_Basic)
{
    QVariantMap args;
    args.insert("device-path", "/dev/sda1");
    args.insert("device-name", "sda1");
    EXPECT_NO_FATAL_FAILURE({
        EncryptParamsInputDialog dlg(args);
        (void)dlg;
    });
}

TEST_F(EncryptParamsInputDialogTest, Constructor_WithMountPoint)
{
    QVariantMap args;
    args.insert("device-path", "/dev/sda1");
    args.insert("device-name", "sda1");
    args.insert("mountpoint", "/home");
    EXPECT_NO_FATAL_FAILURE({
        EncryptParamsInputDialog dlg(args);
        (void)dlg;
    });
}

TEST_F(EncryptParamsInputDialogTest, GetInputs_Basic)
{
    QVariantMap args;
    args.insert("device-path", "/dev/sda1");
    args.insert("device-name", "sda1");
    EncryptParamsInputDialog dlg(args);
    EXPECT_NO_FATAL_FAILURE({
        auto param = dlg.getInputs();
        (void)param;
    });
}

TEST_F(EncryptParamsInputDialogTest, Constructor_EmptyArgs)
{
    QVariantMap args;
    EXPECT_NO_FATAL_FAILURE({
        EncryptParamsInputDialog dlg(args);
        (void)dlg;
    });
}

TEST_F(EncryptParamsInputDialogTest, Constructor_WithExportKey)
{
    stubDConfig(true);
    QVariantMap args;
    args.insert("device-path", "/dev/sda1");
    args.insert("device-name", "sda1");
    EXPECT_NO_FATAL_FAILURE({
        EncryptParamsInputDialog dlg(args);
        (void)dlg;
    });
}
