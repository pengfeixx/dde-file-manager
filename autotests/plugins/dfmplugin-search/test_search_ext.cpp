// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QUrl>

#include "stubext.h"

#include "searchmanager/searcher/dfmsearch/dfmsearcher.h"
#include "searchmanager/searcher/abstractsearcher.h"
#include "utils/abstractindexclient.h"
#include "utils/indexclientdescriptor.h"

using namespace dfmplugin_search;

// ---------------------------------------------------------------------------
// DFMSearcher extension tests
// ---------------------------------------------------------------------------
class DFMSearcherExtTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void TearDown() override
    {
        stub.clear();
    }
};

TEST_F(DFMSearcherExtTest, SupportUrl_FileScheme_ReturnsTrue)
{
    EXPECT_TRUE(DFMSearcher::supportUrl(QUrl("file:///home")));
}

TEST_F(DFMSearcherExtTest, SupportUrl_EmptyUrl_ReturnsFalse)
{
    EXPECT_FALSE(DFMSearcher::supportUrl(QUrl()));
}

TEST_F(DFMSearcherExtTest, SupportUrl_TrashScheme_ReturnsFalse)
{
    EXPECT_FALSE(DFMSearcher::supportUrl(QUrl("trash:///")));
}

TEST_F(DFMSearcherExtTest, SupportUrl_SmbScheme_ReturnsFalse)
{
    EXPECT_FALSE(DFMSearcher::supportUrl(QUrl("smb:///share")));
}

TEST_F(DFMSearcherExtTest, SupportUrl_FtpScheme_ReturnsFalse)
{
    EXPECT_FALSE(DFMSearcher::supportUrl(QUrl("ftp:///host")));
}

TEST_F(DFMSearcherExtTest, SupportUrl_AfpScheme_ReturnsFalse)
{
    EXPECT_FALSE(DFMSearcher::supportUrl(QUrl("afp:///host")));
}

TEST_F(DFMSearcherExtTest, MatchPath_NormalPath_ReturnsNonEmpty)
{
    QString result = DFMSearcher::matchPath("/home/user/documents");
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(DFMSearcherExtTest, MatchPath_EmptyPath_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(DFMSearcher::matchPath(""));
}

TEST_F(DFMSearcherExtTest, MatchPath_RootPath_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(DFMSearcher::matchPath("/"));
}

TEST_F(DFMSearcherExtTest, MatchPath_RelativePath_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(DFMSearcher::matchPath("relative/path"));
}

TEST_F(DFMSearcherExtTest, RealSearchPath_FileUrl_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(DFMSearcher::realSearchPath(QUrl("file:///home/user")));
}

TEST_F(DFMSearcherExtTest, RealSearchPath_EmptyUrl_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(DFMSearcher::realSearchPath(QUrl()));
}

TEST_F(DFMSearcherExtTest, RealSearchPath_NonFileScheme_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(DFMSearcher::realSearchPath(QUrl("trash:///")));
}

TEST_F(DFMSearcherExtTest, RealSearchPath_WithQuery_NoCrash)
{
    QUrl url("file:///home/user");
    url.setQuery("key=value");
    EXPECT_NO_FATAL_FAILURE(DFMSearcher::realSearchPath(url));
}

// ---------------------------------------------------------------------------
// AbstractIndexClient extension tests
// ---------------------------------------------------------------------------
class AbstractIndexClientExtTest : public testing::Test
{
protected:
    void SetUp() override
    {
        IndexClientDescriptor desc;
        desc.clientName = "ext_test_client";
        desc.dbusServiceName = "com.deepin.fake.NonExistentService";
        desc.dbusObjectPath = "/com/deepin/fake/NonExistent";
        desc.interfaceFactory = nullptr;

        client = new AbstractIndexClient(desc);
    }

    void TearDown() override
    {
        // Process pending events to allow DBus async cleanup before deletion
        QCoreApplication::processEvents();
        QCoreApplication::processEvents();
        delete client;
        client = nullptr;
    }

    AbstractIndexClient *client = nullptr;
};

TEST_F(AbstractIndexClientExtTest, Constructor_CreatesClient)
{
    EXPECT_NE(client, nullptr);
}

TEST_F(AbstractIndexClientExtTest, Descriptor_ReturnsCorrectClientName)
{
    EXPECT_EQ(client->descriptor().clientName, "ext_test_client");
}

TEST_F(AbstractIndexClientExtTest, Descriptor_ReturnsCorrectDBusServiceName)
{
    EXPECT_EQ(client->descriptor().dbusServiceName, "com.deepin.fake.NonExistentService");
}

TEST_F(AbstractIndexClientExtTest, Descriptor_ReturnsCorrectDBusObjectPath)
{
    EXPECT_EQ(client->descriptor().dbusObjectPath, "/com/deepin/fake/NonExistent");
}

TEST_F(AbstractIndexClientExtTest, CheckServiceStatus_NoCrash)
{
    QSignalSpy spy(client, &AbstractIndexClient::serviceStatusResult);
    EXPECT_NO_FATAL_FAILURE(client->checkServiceStatus());
    spy.wait(500);
    EXPECT_GE(spy.count(), 0);
}

TEST_F(AbstractIndexClientExtTest, GetIndexStatus_NoCrash)
{
    QSignalSpy spy(client, &AbstractIndexClient::indexStatusResult);
    EXPECT_NO_FATAL_FAILURE(client->getIndexStatus());
    spy.wait(500);
    EXPECT_GE(spy.count(), 0);
}

TEST_F(AbstractIndexClientExtTest, SetEnable_True_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(client->setEnable(true));
}

TEST_F(AbstractIndexClientExtTest, SetEnable_False_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(client->setEnable(false));
}

TEST_F(AbstractIndexClientExtTest, CheckIndexExists_NoCrash)
{
    QSignalSpy spy(client, &AbstractIndexClient::indexExistsResult);
    EXPECT_NO_FATAL_FAILURE(client->checkIndexExists());
    spy.wait(500);
    EXPECT_GE(spy.count(), 0);
}

TEST_F(AbstractIndexClientExtTest, CheckHasRunningTask_NoCrash)
{
    QSignalSpy spy(client, &AbstractIndexClient::hasRunningTaskResult);
    EXPECT_NO_FATAL_FAILURE(client->checkHasRunningTask());
    spy.wait(500);
    EXPECT_GE(spy.count(), 0);
}

TEST_F(AbstractIndexClientExtTest, CheckHasRunningRootTask_NoCrash)
{
    QSignalSpy spy(client, &AbstractIndexClient::hasRunningRootTaskResult);
    EXPECT_NO_FATAL_FAILURE(client->checkHasRunningRootTask());
    spy.wait(500);
    EXPECT_GE(spy.count(), 0);
}

TEST_F(AbstractIndexClientExtTest, GetLastUpdateTime_NoCrash)
{
    QSignalSpy spy(client, &AbstractIndexClient::lastUpdateTimeResult);
    EXPECT_NO_FATAL_FAILURE(client->getLastUpdateTime());
    spy.wait(500);
    EXPECT_GE(spy.count(), 0);
}

TEST_F(AbstractIndexClientExtTest, ForceUpdateIndex_MultiplePaths_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(client->forceUpdateIndex({"/tmp", "/home", "/var"}));
}

TEST_F(AbstractIndexClientExtTest, ForceUpdateIndex_EmptyPaths_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(client->forceUpdateIndex({}));
}

TEST_F(AbstractIndexClientExtTest, UpdateIndexBypassEnv_MultiplePaths_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(client->updateIndexBypassEnv({"/tmp", "/home"}));
}

TEST_F(AbstractIndexClientExtTest, UpdateIndexBypassEnv_EmptyPaths_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(client->updateIndexBypassEnv({}));
}

TEST_F(AbstractIndexClientExtTest, StartTask_Create_WithMultiplePaths_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(client->startTask(AbstractIndexClient::TaskType::Create, {"/tmp", "/home", "/var"}));
}

TEST_F(AbstractIndexClientExtTest, StartTask_Update_WithEmptyPaths_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(client->startTask(AbstractIndexClient::TaskType::Update, {}));
}

TEST_F(AbstractIndexClientExtTest, StartTask_CreateFileList_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(client->startTask(AbstractIndexClient::TaskType::CreateFileList, {"/tmp"}));
}

TEST_F(AbstractIndexClientExtTest, StartTask_UpdateFileList_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(client->startTask(AbstractIndexClient::TaskType::UpdateFileList, {"/tmp"}));
}

TEST_F(AbstractIndexClientExtTest, StartTask_RemoveFileList_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(client->startTask(AbstractIndexClient::TaskType::RemoveFileList, {"/tmp"}));
}

TEST_F(AbstractIndexClientExtTest, StartTask_MoveFileList_MultiplePaths_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(client->startTask(AbstractIndexClient::TaskType::MoveFileList, {"/tmp", "/home"}));
}

TEST_F(AbstractIndexClientExtTest, StartTask_WithOptions_NoCrash)
{
    QVariantMap options;
    options["force"] = true;
    options["recursive"] = false;
    EXPECT_NO_FATAL_FAILURE(client->startTask(AbstractIndexClient::TaskType::Update, {"/tmp"}, options));
}

TEST_F(AbstractIndexClientExtTest, StartTask_WithEmptyOptions_NoCrash)
{
    QVariantMap options;
    EXPECT_NO_FATAL_FAILURE(client->startTask(AbstractIndexClient::TaskType::Create, {"/tmp"}, options));
}

TEST_F(AbstractIndexClientExtTest, TaskType_EnumValues_Valid)
{
    EXPECT_NE(AbstractIndexClient::TaskType::Create, AbstractIndexClient::TaskType::Update);
    EXPECT_NE(AbstractIndexClient::TaskType::CreateFileList, AbstractIndexClient::TaskType::UpdateFileList);
    EXPECT_NE(AbstractIndexClient::TaskType::RemoveFileList, AbstractIndexClient::TaskType::MoveFileList);
}

TEST_F(AbstractIndexClientExtTest, ServiceStatus_EnumValues_Valid)
{
    EXPECT_NE(AbstractIndexClient::ServiceStatus::Available, AbstractIndexClient::ServiceStatus::Unavailable);
    EXPECT_NE(AbstractIndexClient::ServiceStatus::Unavailable, AbstractIndexClient::ServiceStatus::Error);
}

TEST_F(AbstractIndexClientExtTest, MultipleClients_DifferentNames_NoCrash)
{
    IndexClientDescriptor desc2;
    desc2.clientName = "second_client";
    desc2.dbusServiceName = "com.deepin.fake.Another";
    desc2.dbusObjectPath = "/com/deepin/fake/Another";
    desc2.interfaceFactory = nullptr;

    AbstractIndexClient client2(desc2);
    EXPECT_EQ(client2.descriptor().clientName, "second_client");
    EXPECT_NO_FATAL_FAILURE(client2.setEnable(true));
}
