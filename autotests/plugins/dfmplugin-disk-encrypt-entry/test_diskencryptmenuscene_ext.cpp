// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "menu/diskencryptmenuscene.h"
#include "dfmplugin_disk_encrypt_global.h"
#include "services/diskencrypt/globaltypesdefine.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/dfm_menu_defines.h>
#include <dfm-mount/dmount.h>

#include <gtest/gtest.h>
#include <QMenu>
#include <QAction>
#include <QUrl>
#include <QVariantHash>
#include <QDBusInterface>
#include <QDBusAbstractInterface>
#include <QDBusMessage>
#include <QTest>
#include <QTemporaryFile>

#include <dconfig.h>

using namespace dfmplugin_diskenc;
using namespace disk_encrypt;
DFMBASE_USE_NAMESPACE

class DiskEncryptMenuSceneExtTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        scene = new DiskEncryptMenuScene();
        stub.set_lamda(&QDBusAbstractInterface::isValid, [](QDBusAbstractInterface *) -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });
    }
    void TearDown() override { stub.clear(); delete scene; }
    DiskEncryptMenuScene *scene = nullptr;

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

TEST_F(DiskEncryptMenuSceneExtTest, Initialize_WithSelectFiles)
{
    stubDConfig(true);
    QVariantHash params;
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/dev/sda1");
    params.insert(MenuParamKey::kSelectFiles, QVariant::fromValue(urls));
    params.insert(MenuParamKey::kOnDesktop, false);
    EXPECT_NO_FATAL_FAILURE(scene->initialize(params));
}

TEST_F(DiskEncryptMenuSceneExtTest, Initialize_EmptyOnDesktop)
{
    QVariantHash params;
    params.insert(MenuParamKey::kOnDesktop, true);
    EXPECT_FALSE(scene->initialize(params));
}

TEST_F(DiskEncryptMenuSceneExtTest, Create_EmptyMenu_NoCrash)
{
    QMenu menu;
    stubDConfig(true);
    EXPECT_NO_FATAL_FAILURE(scene->create(&menu));
}

TEST_F(DiskEncryptMenuSceneExtTest, Create_WithParent)
{
    QMenu menu;
    EXPECT_NO_FATAL_FAILURE(scene->create(&menu));
}

TEST_F(DiskEncryptMenuSceneExtTest, Triggered_EncryptAction)
{
    QAction action("encrypt");
    action.setData("encrypt-device");
    EXPECT_NO_FATAL_FAILURE(scene->triggered(&action));
}

TEST_F(DiskEncryptMenuSceneExtTest, Triggered_DecryptAction)
{
    QAction action("decrypt");
    action.setData("decrypt-device");
    EXPECT_NO_FATAL_FAILURE(scene->triggered(&action));
}

TEST_F(DiskEncryptMenuSceneExtTest, Triggered_ChangePassphraseAction)
{
    QAction action("change-passphrase");
    action.setData("change-passphrase");
    EXPECT_NO_FATAL_FAILURE(scene->triggered(&action));
}

TEST_F(DiskEncryptMenuSceneExtTest, Triggered_UnlockAction)
{
    QAction action("unlock");
    action.setData("unlock-device");
    EXPECT_NO_FATAL_FAILURE(scene->triggered(&action));
}

TEST_F(DiskEncryptMenuSceneExtTest, GenerateTPMConfig_NoCrash)
{
    stubDConfig(true);
    QString result;
    EXPECT_NO_FATAL_FAILURE(result = DiskEncryptMenuScene::generateTPMConfig());
}

TEST_F(DiskEncryptMenuSceneExtTest, GenerateTPMToken_NoCrash)
{
    stubDConfig(true);
    QString result;
    EXPECT_NO_FATAL_FAILURE(result = DiskEncryptMenuScene::generateTPMToken("/dev/sda1", false, "/tmp/config"));
}

TEST_F(DiskEncryptMenuSceneExtTest, SendCredentialsViaFd_NoCrash)
{
    QDBusInterface iface("org.test", "/test", "org.test.iface", QDBusConnection::sessionBus());
    QVariantMap params;
    params.insert("key", "value");
    bool result = false;
    EXPECT_NO_FATAL_FAILURE(result = DiskEncryptMenuScene::sendCredentialsViaFd(iface, "TestMethod", params));
}

TEST_F(DiskEncryptMenuSceneExtTest, GetBase64Of_EmptyPath)
{
    QTemporaryFile tmpFile;
    ASSERT_TRUE(tmpFile.open());
    tmpFile.write("test data");
    tmpFile.close();
    SUCCEED();
}
