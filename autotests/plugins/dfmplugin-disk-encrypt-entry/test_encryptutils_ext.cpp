// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "utils/encryptutils.h"
#include "dfmplugin_disk_encrypt_global.h"

#include <dfm-mount/dmount.h>

#include <gtest/gtest.h>

#include <QApplication>
#include <QDBusInterface>
#include <QDBusAbstractInterface>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QTemporaryDir>
#include <QFile>
#include <QVariantMap>

#include <dconfig.h>

using namespace dfmplugin_diskenc;

class EncryptUtilsExtTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.set_lamda(&QDBusAbstractInterface::isValid, [](QDBusAbstractInterface *) -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });
    }
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

TEST_F(EncryptUtilsExtTest, EnableEncrypt_True)
{
    stubDConfig(true);
    EXPECT_TRUE(config_utils::enableEncrypt());
}

TEST_F(EncryptUtilsExtTest, EnableEncrypt_False)
{
    stubDConfig(false);
    EXPECT_FALSE(config_utils::enableEncrypt());
}

TEST_F(EncryptUtilsExtTest, EnableAlgoFromDConfig_True)
{
    stubDConfig(true);
    EXPECT_TRUE(config_utils::enableAlgoFromDConfig());
}

TEST_F(EncryptUtilsExtTest, EnableAlgoFromDConfig_False)
{
    stubDConfig(false);
    EXPECT_FALSE(config_utils::enableAlgoFromDConfig());
}

TEST_F(EncryptUtilsExtTest, UseOverlayDMMode_True)
{
    stubDConfig(true);
    EXPECT_TRUE(config_utils::useOverlayDMMode());
}

TEST_F(EncryptUtilsExtTest, UseOverlayDMMode_False)
{
    stubDConfig(false);
    EXPECT_FALSE(config_utils::useOverlayDMMode());
}

TEST_F(EncryptUtilsExtTest, TpmAlgoFromDConfig_ReturnsValues)
{
    stubDConfig(QString("sm3_256"));
    QString sh, sk, ph, pk, mh, mk, pcr, pcrbank;
    EXPECT_TRUE(config_utils::tpmAlgoFromDConfig(&sh, &sk, &ph, &pk, &mh, &mk, &pcr, &pcrbank));
}

TEST_F(EncryptUtilsExtTest, CheckTPM_InvalidDBus)
{
    bool authFailed = false;
    EXPECT_EQ(tpm_utils::checkTPM(&authFailed), -1);
}

TEST_F(EncryptUtilsExtTest, CheckTPM_NoAuthFailed)
{
    EXPECT_EQ(tpm_utils::checkTPM(), -1);
}

TEST_F(EncryptUtilsExtTest, CheckTPMLockoutStatus_InvalidDBus)
{
    EXPECT_EQ(tpm_utils::checkTPMLockoutStatus(), -1);
}

TEST_F(EncryptUtilsExtTest, OwnerAuthStatus_InvalidDBus)
{
    EXPECT_EQ(tpm_utils::ownerAuthStatus(), -1);
}

TEST_F(EncryptUtilsExtTest, IsSupportAlgoByTPM_InvalidDBus)
{
    bool support = false;
    EXPECT_EQ(tpm_utils::isSupportAlgoByTPM("aes", &support), -1);
}

TEST_F(EncryptUtilsExtTest, GetRandomByTPM_InvalidDBus)
{
    QString output;
    EXPECT_EQ(tpm_utils::getRandomByTPM(16, &output), -1);
}

TEST_F(EncryptUtilsExtTest, EncryptByTPM_InvalidDBus)
{
    QVariantMap map;
    EXPECT_EQ(tpm_utils::encryptByTPM(map), -1);
}

TEST_F(EncryptUtilsExtTest, DecryptByTPM_InvalidDBus)
{
    QString psw;
    EXPECT_EQ(tpm_utils::decryptByTPM(QVariantMap(), &psw), -1);
}

TEST_F(EncryptUtilsExtTest, GenPassphraseFromTPM_NonBlock_InvalidDBus)
{
    QString passphrase;
    EXPECT_NE(tpm_passphrase_utils::genPassphraseFromTPM_NonBlock("/dev/sda1", "", &passphrase), 0);
}

TEST_F(EncryptUtilsExtTest, GetPassphraseFromTPM_NonBlock_InvalidDBus)
{
    QString result = tpm_passphrase_utils::getPassphraseFromTPM_NonBlock("/dev/sda1", "");
    EXPECT_TRUE(result.isEmpty());
}
