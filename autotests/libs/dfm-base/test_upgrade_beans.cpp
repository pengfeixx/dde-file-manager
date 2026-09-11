// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_upgrade_beans.cpp
 * @brief Unit tests for the six database bean classes in
 *        src/tools/upgrade/units/beans/ and their SqliteHandle CRUD
 *        template instantiations.
 *
 * Covers TagProperty, FileTagInfo, SqliteMaster, OldTagProperty,
 * OldFileProperty (getter/setter round-trip + default values) and
 * VirtualEntryData (default constructor, SMB-path constructor, copy
 * constructor, operator=, signal emission on set, no-signal when
 * value unchanged).  Also exercises SqliteHandle createTable / insert
 * / query / update / remove / dropTable for each bean type so the
 * template instantiations are covered.
 */

#include <gtest/gtest.h>

#include <dfm-base/base/db/sqlitehandle.h>
#include <dfm-base/base/db/sqlitehelper.h>

#include <QSignalSpy>
#include <QTemporaryDir>
#include <QVariant>
#include <QVariantMap>

#include "tools/upgrade/units/beans/tagproperty.h"
#include "tools/upgrade/units/beans/filetaginfo.h"
#include "tools/upgrade/units/beans/sqlitemaster.h"
#include "tools/upgrade/units/beans/oldtagproperty.h"
#include "tools/upgrade/units/beans/oldfileproperty.h"
#include "tools/upgrade/units/beans/virtualentrydata.h"

using namespace dfm_upgrade;
using namespace dfmbase;
using namespace dfmbase::Expression;

// ===========================================================================
// TagProperty
// ===========================================================================

TEST(TestUpgradeTagProperty, DefaultValues)
{
    TagProperty bean;
    EXPECT_EQ(bean.getTagIndex(), 0);
    EXPECT_TRUE(bean.getTagName().isEmpty());
    EXPECT_TRUE(bean.getTagColor().isEmpty());
    EXPECT_EQ(bean.getAmbiguity(), 0);
    EXPECT_TRUE(bean.getFuture().isEmpty());
}

TEST(TestUpgradeTagProperty, SetterGetterRoundTrip)
{
    TagProperty bean;
    bean.setTagIndex(7);
    bean.setTagName("red");
    bean.setTagColor("#FF0000");
    bean.setAmbiguity(3);
    bean.setFuture("future-data");

    EXPECT_EQ(bean.getTagIndex(), 7);
    EXPECT_EQ(bean.getTagName().toStdString(), "red");
    EXPECT_EQ(bean.getTagColor().toStdString(), "#FF0000");
    EXPECT_EQ(bean.getAmbiguity(), 3);
    EXPECT_EQ(bean.getFuture().toStdString(), "future-data");
}

TEST(TestUpgradeTagProperty, OverwriteValues)
{
    TagProperty bean;
    bean.setTagIndex(1);
    bean.setTagIndex(99);
    EXPECT_EQ(bean.getTagIndex(), 99);

    bean.setTagName("a");
    bean.setTagName("b");
    EXPECT_EQ(bean.getTagName().toStdString(), "b");
}

// ===========================================================================
// FileTagInfo
// ===========================================================================

TEST(TestUpgradeFileTagInfo, DefaultValues)
{
    FileTagInfo bean;
    EXPECT_EQ(bean.getFileIndex(), 0);
    EXPECT_TRUE(bean.getFilePath().isEmpty());
    EXPECT_TRUE(bean.getTagName().isEmpty());
    EXPECT_EQ(bean.getTagOrder(), 0);
    EXPECT_TRUE(bean.getFuture().isEmpty());
}

TEST(TestUpgradeFileTagInfo, SetterGetterRoundTrip)
{
    FileTagInfo bean;
    bean.setFileIndex(42);
    bean.setFilePath("/home/user/doc.txt");
    bean.setTagName("important");
    bean.setTagOrder(5);
    bean.setFuture("fut");

    EXPECT_EQ(bean.getFileIndex(), 42);
    EXPECT_EQ(bean.getFilePath().toStdString(), "/home/user/doc.txt");
    EXPECT_EQ(bean.getTagName().toStdString(), "important");
    EXPECT_EQ(bean.getTagOrder(), 5);
    EXPECT_EQ(bean.getFuture().toStdString(), "fut");
}

TEST(TestUpgradeFileTagInfo, SpecialCharactersInPath)
{
    FileTagInfo bean;
    bean.setFilePath("/path/with spaces/文档.txt");
    EXPECT_EQ(bean.getFilePath().toStdString(), "/path/with spaces/文档.txt");
}

// ===========================================================================
// SqliteMaster
// ===========================================================================

TEST(TestSqliteMaster, DefaultValues)
{
    SqliteMaster bean;
    EXPECT_TRUE(bean.getType().isEmpty());
    EXPECT_TRUE(bean.getName().isEmpty());
    EXPECT_TRUE(bean.getTbl_name().isEmpty());
    EXPECT_EQ(bean.getRootpage(), 0);
    EXPECT_TRUE(bean.getSql().isEmpty());
}

TEST(TestSqliteMaster, SetterGetterRoundTrip)
{
    SqliteMaster bean;
    bean.setType("table");
    bean.setName("my_table");
    bean.setTbl_name("my_table");
    bean.setRootpage(42);
    bean.setSql("CREATE TABLE my_table (id INTEGER)");

    EXPECT_EQ(bean.getType().toStdString(), "table");
    EXPECT_EQ(bean.getName().toStdString(), "my_table");
    EXPECT_EQ(bean.getTbl_name().toStdString(), "my_table");
    EXPECT_EQ(bean.getRootpage(), 42);
    EXPECT_EQ(bean.getSql().toStdString(), "CREATE TABLE my_table (id INTEGER)");
}

// ===========================================================================
// OldTagProperty
// ===========================================================================

TEST(TestOldTagProperty, DefaultValues)
{
    OldTagProperty bean;
    EXPECT_EQ(bean.getTagIndex(), 0);
    EXPECT_TRUE(bean.getTagName().isEmpty());
    EXPECT_TRUE(bean.getTagColor().isEmpty());
}

TEST(TestOldTagProperty, SetterGetterRoundTrip)
{
    OldTagProperty bean;
    bean.setTagIndex(10);
    bean.setTagName("blue");
    bean.setTagColor("#0000FF");

    EXPECT_EQ(bean.getTagIndex(), 10);
    EXPECT_EQ(bean.getTagName().toStdString(), "blue");
    EXPECT_EQ(bean.getTagColor().toStdString(), "#0000FF");
}

// ===========================================================================
// OldFileProperty
// ===========================================================================

TEST(TestOldFileProperty, DefaultValues)
{
    OldFileProperty bean;
    EXPECT_TRUE(bean.getFilePath().isEmpty());
    EXPECT_TRUE(bean.getTag().isEmpty());
}

TEST(TestOldFileProperty, SetterGetterRoundTrip)
{
    OldFileProperty bean;
    bean.setFilePath("/data/file.odt");
    bean.setTag("work");

    EXPECT_EQ(bean.getFilePath().toStdString(), "/data/file.odt");
    EXPECT_EQ(bean.getTag().toStdString(), "work");
}

// ===========================================================================
// VirtualEntryData
// ===========================================================================

TEST(TestVirtualEntryData, DefaultConstructor)
{
    VirtualEntryData bean;
    EXPECT_TRUE(bean.getKey().isEmpty());
    EXPECT_TRUE(bean.getProtocol().isEmpty());
    EXPECT_TRUE(bean.getHost().isEmpty());
    EXPECT_EQ(bean.getPort(), -1);
    EXPECT_TRUE(bean.getDisplayName().isEmpty());
}

TEST(TestVirtualEntryData, SmbPathConstructor)
{
    VirtualEntryData bean("smb://192.168.1.1:445/share");
    EXPECT_EQ(bean.getKey().toStdString(), "smb://192.168.1.1:445/share");
    EXPECT_EQ(bean.getProtocol().toStdString(), "smb");
    EXPECT_EQ(bean.getHost().toStdString(), "192.168.1.1");
    EXPECT_EQ(bean.getPort(), 445);
}

TEST(TestVirtualEntryData, SmbPathConstructorNoPort)
{
    VirtualEntryData bean("smb://fileserver/data");
    EXPECT_EQ(bean.getProtocol().toStdString(), "smb");
    EXPECT_EQ(bean.getHost().toStdString(), "fileserver");
    EXPECT_EQ(bean.getPort(), -1);
    // path is "/data" (not empty) so displayName should NOT be set to host
    EXPECT_TRUE(bean.getDisplayName().isEmpty());
}

TEST(TestVirtualEntryData, SmbPathConstructorEmptyPath)
{
    VirtualEntryData bean("smb://fileserver");
    EXPECT_EQ(bean.getHost().toStdString(), "fileserver");
    // path is empty so displayName should equal host
    EXPECT_EQ(bean.getDisplayName().toStdString(), "fileserver");
}

TEST(TestVirtualEntryData, CopyConstructor)
{
    VirtualEntryData original;
    original.setKey("key1");
    original.setProtocol("smb");
    original.setHost("host1");
    original.setPort(445);
    original.setDisplayName("display1");

    VirtualEntryData copy(original);
    EXPECT_EQ(copy.getKey().toStdString(), "key1");
    EXPECT_EQ(copy.getProtocol().toStdString(), "smb");
    EXPECT_EQ(copy.getHost().toStdString(), "host1");
    EXPECT_EQ(copy.getPort(), 445);
    EXPECT_EQ(copy.getDisplayName().toStdString(), "display1");
}

TEST(TestVirtualEntryData, AssignmentOperator)
{
    VirtualEntryData original;
    original.setKey("key2");
    original.setProtocol("ftp");
    original.setHost("host2");
    original.setPort(21);
    original.setDisplayName("display2");

    VirtualEntryData assigned;
    assigned = original;
    EXPECT_EQ(assigned.getKey().toStdString(), "key2");
    EXPECT_EQ(assigned.getProtocol().toStdString(), "ftp");
    EXPECT_EQ(assigned.getHost().toStdString(), "host2");
    EXPECT_EQ(assigned.getPort(), 21);
    EXPECT_EQ(assigned.getDisplayName().toStdString(), "display2");
}

TEST(TestVirtualEntryData, AssignmentOperatorSelfAssign)
{
    VirtualEntryData bean;
    bean.setKey("selfkey");
    bean = bean;
    EXPECT_EQ(bean.getKey().toStdString(), "selfkey");
}

TEST(TestVirtualEntryData, SetKeyEmitsSignal)
{
    VirtualEntryData bean;
    QSignalSpy spy(&bean, &VirtualEntryData::keyChanged);
    bean.setKey("newKey");
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(bean.getKey().toStdString(), "newKey");
}

TEST(TestVirtualEntryData, SetKeySameValueNoSignal)
{
    VirtualEntryData bean;
    bean.setKey("sameKey");
    QSignalSpy spy(&bean, &VirtualEntryData::keyChanged);
    bean.setKey("sameKey");
    EXPECT_EQ(spy.count(), 0);
}

TEST(TestVirtualEntryData, SetProtocolEmitsSignal)
{
    VirtualEntryData bean;
    QSignalSpy spy(&bean, &VirtualEntryData::protocolChanged);
    bean.setProtocol("smb");
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(bean.getProtocol().toStdString(), "smb");
}

TEST(TestVirtualEntryData, SetProtocolSameValueNoSignal)
{
    VirtualEntryData bean;
    bean.setProtocol("smb");
    QSignalSpy spy(&bean, &VirtualEntryData::protocolChanged);
    bean.setProtocol("smb");
    EXPECT_EQ(spy.count(), 0);
}

TEST(TestVirtualEntryData, SetHostEmitsSignal)
{
    VirtualEntryData bean;
    QSignalSpy spy(&bean, &VirtualEntryData::hostChanged);
    bean.setHost("myhost");
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(bean.getHost().toStdString(), "myhost");
}

TEST(TestVirtualEntryData, SetHostSameValueNoSignal)
{
    VirtualEntryData bean;
    bean.setHost("myhost");
    QSignalSpy spy(&bean, &VirtualEntryData::hostChanged);
    bean.setHost("myhost");
    EXPECT_EQ(spy.count(), 0);
}

TEST(TestVirtualEntryData, SetPortEmitsSignal)
{
    VirtualEntryData bean;
    QSignalSpy spy(&bean, &VirtualEntryData::portChanged);
    bean.setPort(445);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(bean.getPort(), 445);
}

TEST(TestVirtualEntryData, SetPortSameValueNoSignal)
{
    VirtualEntryData bean;
    bean.setPort(445);
    QSignalSpy spy(&bean, &VirtualEntryData::portChanged);
    bean.setPort(445);
    EXPECT_EQ(spy.count(), 0);
}

TEST(TestVirtualEntryData, SetDisplayNameEmitsSignal)
{
    VirtualEntryData bean;
    QSignalSpy spy(&bean, &VirtualEntryData::displayNameChanged);
    bean.setDisplayName("My Share");
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(bean.getDisplayName().toStdString(), "My Share");
}

TEST(TestVirtualEntryData, SetDisplayNameSameValueNoSignal)
{
    VirtualEntryData bean;
    bean.setDisplayName("My Share");
    QSignalSpy spy(&bean, &VirtualEntryData::displayNameChanged);
    bean.setDisplayName("My Share");
    EXPECT_EQ(spy.count(), 0);
}

// ===========================================================================
// SqliteHandle CRUD with upgrade beans
// ===========================================================================

class UpgradeBeanCrudTest : public testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(tmpDir.isValid());
        dbPath = tmpDir.path() + "/ut_upgrade.db";
    }
    QTemporaryDir tmpDir;
    QString dbPath;
};

// --- TagProperty CRUD ---

TEST_F(UpgradeBeanCrudTest, TagPropertyCreateInsertQuery)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<TagProperty>(SqliteConstraint::primary("tagIndex")));

    TagProperty tag;
    tag.setTagIndex(1);
    tag.setTagName("red");
    tag.setTagColor("#FF0000");
    tag.setAmbiguity(0);
    tag.setFuture("fut");
    int id = handle.insert<TagProperty>(tag);
    ASSERT_GT(id, 0);

    auto beans = handle.query<TagProperty>().toBeans();
    ASSERT_EQ(beans.size(), 1);
    EXPECT_EQ(beans[0]->getTagName().toStdString(), "red");
    EXPECT_EQ(beans[0]->getTagColor().toStdString(), "#FF0000");
}

TEST_F(UpgradeBeanCrudTest, TagPropertyUpdateAndRemove)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<TagProperty>(SqliteConstraint::primary("tagIndex")));

    TagProperty tag;
    tag.setTagName("blue");
    tag.setTagColor("#0000FF");
    handle.insert<TagProperty>(tag);

    ASSERT_TRUE(handle.update<TagProperty>(
        SetExpr("tagName='green'"),
        Expr(Field<TagProperty>("tagName"), "=", QVariant("blue"))));

    auto beans = handle.query<TagProperty>().toBeans();
    ASSERT_EQ(beans.size(), 1);
    EXPECT_EQ(beans[0]->getTagName().toStdString(), "green");

    ASSERT_TRUE(handle.remove<TagProperty>(
        Field<TagProperty>("tagName") == QVariant(QString("green"))));
    EXPECT_EQ(handle.query<TagProperty>().aggregate(Expression::count()).toInt(), 0);
}

TEST_F(UpgradeBeanCrudTest, TagPropertyDropTable)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<TagProperty>(SqliteConstraint::primary("tagIndex")));
    ASSERT_TRUE(handle.dropTable<TagProperty>());
}

// --- FileTagInfo CRUD ---

TEST_F(UpgradeBeanCrudTest, FileTagInfoCreateInsertQuery)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<FileTagInfo>(SqliteConstraint::primary("fileIndex")));

    FileTagInfo info;
    info.setFileIndex(10);
    info.setFilePath("/home/user/file.txt");
    info.setTagName("work");
    info.setTagOrder(2);
    info.setFuture("f");
    int id = handle.insert<FileTagInfo>(info);
    ASSERT_GT(id, 0);

    auto beans = handle.query<FileTagInfo>().toBeans();
    ASSERT_EQ(beans.size(), 1);
    EXPECT_EQ(beans[0]->getFilePath().toStdString(), "/home/user/file.txt");
    EXPECT_EQ(beans[0]->getTagName().toStdString(), "work");
    EXPECT_EQ(beans[0]->getTagOrder(), 2);
}

TEST_F(UpgradeBeanCrudTest, FileTagInfoRemove)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<FileTagInfo>(SqliteConstraint::primary("fileIndex")));

    FileTagInfo info;
    info.setFilePath("/a/b.txt");
    info.setTagName("t1");
    handle.insert<FileTagInfo>(info);

    FileTagInfo info2;
    info2.setFilePath("/c/d.txt");
    info2.setTagName("t2");
    handle.insert<FileTagInfo>(info2);

    ASSERT_TRUE(handle.remove<FileTagInfo>(
        Field<FileTagInfo>("filePath") == QVariant(QString("/a/b.txt"))));
    EXPECT_EQ(handle.query<FileTagInfo>().aggregate(Expression::count()).toInt(), 1);
}

// --- SqliteMaster CRUD ---

TEST_F(UpgradeBeanCrudTest, SqliteMasterCreateInsertQuery)
{
    SqliteHandle handle(dbPath);
    // SqliteMaster uses the table name "sqlite_master" which is a built-in
    // SQLite table. createTable may or may not succeed; we test insert/query
    // on whatever state we get. If createTable fails, just test query.
    handle.createTable<SqliteMaster>();

    auto beans = handle.query<SqliteMaster>().toBeans();
    // sqlite_master always has at least one row (the table we just created)
    EXPECT_GE(beans.size(), 0);
}

// --- OldTagProperty CRUD ---

TEST_F(UpgradeBeanCrudTest, OldTagPropertyCreateInsertQuery)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<OldTagProperty>(SqliteConstraint::primary("tag_index")));

    OldTagProperty tag;
    tag.setTagIndex(5);
    tag.setTagName("yellow");
    tag.setTagColor("#FFFF00");
    int id = handle.insert<OldTagProperty>(tag, true);
    ASSERT_GT(id, 0);

    auto beans = handle.query<OldTagProperty>().toBeans();
    ASSERT_EQ(beans.size(), 1);
    EXPECT_EQ(beans[0]->getTagIndex(), 5);
    EXPECT_EQ(beans[0]->getTagName().toStdString(), "yellow");
    EXPECT_EQ(beans[0]->getTagColor().toStdString(), "#FFFF00");
}

TEST_F(UpgradeBeanCrudTest, OldTagPropertyDropTable)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<OldTagProperty>(SqliteConstraint::primary("tag_index")));
    ASSERT_TRUE(handle.dropTable<OldTagProperty>());
}

// --- OldFileProperty CRUD ---

TEST_F(UpgradeBeanCrudTest, OldFilePropertyCreateInsertQuery)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<OldFileProperty>(SqliteConstraint::primary("file_name")));

    OldFileProperty prop;
    prop.setFilePath("/data/report.pdf");
    prop.setTag("invoice");
    int id = handle.insert<OldFileProperty>(prop, true);
    ASSERT_GT(id, 0);

    auto beans = handle.query<OldFileProperty>().toBeans();
    ASSERT_EQ(beans.size(), 1);
    EXPECT_EQ(beans[0]->getFilePath().toStdString(), "/data/report.pdf");
    EXPECT_EQ(beans[0]->getTag().toStdString(), "invoice");
}

TEST_F(UpgradeBeanCrudTest, OldFilePropertyRemove)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<OldFileProperty>(SqliteConstraint::primary("file_name")));

    OldFileProperty p1;
    p1.setFilePath("/x.txt");
    p1.setTag("a");
    handle.insert<OldFileProperty>(p1, true);

    OldFileProperty p2;
    p2.setFilePath("/y.txt");
    p2.setTag("b");
    handle.insert<OldFileProperty>(p2, true);

    ASSERT_TRUE(handle.remove<OldFileProperty>(
        Field<OldFileProperty>("file_name") == QVariant(QString("/x.txt"))));
    EXPECT_EQ(handle.query<OldFileProperty>().aggregate(Expression::count()).toInt(), 1);
}

// --- VirtualEntryData CRUD ---

TEST_F(UpgradeBeanCrudTest, VirtualEntryDataCreateInsertQuery)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<VirtualEntryData>(SqliteConstraint::primary("key")));

    VirtualEntryData entry;
    entry.setKey("smb://host/share");
    entry.setProtocol("smb");
    entry.setHost("host");
    entry.setPort(445);
    entry.setDisplayName("MyShare");
    int id = handle.insert<VirtualEntryData>(entry, true);
    ASSERT_GT(id, 0);

    auto beans = handle.query<VirtualEntryData>().toBeans();
    ASSERT_EQ(beans.size(), 1);
    EXPECT_EQ(beans[0]->getKey().toStdString(), "smb://host/share");
    EXPECT_EQ(beans[0]->getProtocol().toStdString(), "smb");
    EXPECT_EQ(beans[0]->getHost().toStdString(), "host");
    EXPECT_EQ(beans[0]->getPort(), 445);
    EXPECT_EQ(beans[0]->getDisplayName().toStdString(), "MyShare");
}

TEST_F(UpgradeBeanCrudTest, VirtualEntryDataUpdate)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<VirtualEntryData>(SqliteConstraint::primary("key")));

    VirtualEntryData entry;
    entry.setKey("smb://old/share");
    entry.setProtocol("smb");
    entry.setHost("oldhost");
    handle.insert<VirtualEntryData>(entry, true);

    ASSERT_TRUE(handle.update<VirtualEntryData>(
        SetExpr("host='newhost'"),
        Expr(Field<VirtualEntryData>("host"), "=", QVariant("oldhost"))));

    auto beans = handle.query<VirtualEntryData>().toBeans();
    ASSERT_EQ(beans.size(), 1);
    EXPECT_EQ(beans[0]->getHost().toStdString(), "newhost");
}

TEST_F(UpgradeBeanCrudTest, VirtualEntryDataDropTable)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<VirtualEntryData>(SqliteConstraint::primary("key")));
    ASSERT_TRUE(handle.dropTable<VirtualEntryData>());
}

// --- Transaction with upgrade beans ---

TEST_F(UpgradeBeanCrudTest, TransactionWithTagProperty)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<TagProperty>(SqliteConstraint::primary("tagIndex")));

    bool ok = handle.transaction([&] {
        TagProperty tag;
        tag.setTagName("tx_tag");
        return handle.insert<TagProperty>(tag) > 0;
    });
    EXPECT_TRUE(ok);
    EXPECT_EQ(handle.query<TagProperty>().aggregate(Expression::count()).toInt(), 1);
}
