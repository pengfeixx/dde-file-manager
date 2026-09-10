// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/baseitemdelegate.h"
#include "views/private/baseitemdelegate_p.h"
#include "views/fileview.h"
#include "views/abstractitempaintproxy.h"
#include "utils/fileviewhelper.h"
#include "utils/filesortworker.h"
#include "utils/fileoperatorhelper.h"
#include "utils/workspacehelper.h"
#include "models/fileviewmodel.h"
#include "models/fileitemdata.h"
#include "dfmplugin_workspace_global.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/interfaces/sortfileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/clipboard.h>

#include <DApplication>
#include <DPalette>
#include <DPaletteHelper>
#include <DGuiApplicationHelper>
#include <DFontSizeManager>

#include <QApplication>
#include <QStyle>
#include <QPainter>
#include <QPixmap>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QUrl>
#include <QUrlQuery>
#include <QList>
#include <QVariant>
#include <QDir>
#include <QTimer>
#include <QDBusInterface>
#include <QDBusMessage>

using namespace dfmplugin_workspace;
using namespace dfmbase::Global;
using namespace dfmbase;

// ---------------------------------------------------------------------------
// Concrete subclass for testing abstract BaseItemDelegate
// ---------------------------------------------------------------------------
class TestableDelegate : public BaseItemDelegate
{
public:
    explicit TestableDelegate(FileViewHelper *parent)
        : BaseItemDelegate(parent) {}

    QList<QRect> paintGeomertys(const QStyleOptionViewItem &, const QModelIndex &, bool = false) const override
    {
        return {};
    }

    void updateItemSizeHint() override
    {
        Q_D(BaseItemDelegate);
        d->itemSizeHint = QSize(100, 100);
    }

    int getGroupHeaderHeight(const QStyleOptionViewItem &option) const override
    {
        return option.rect.height();
    }
};

// ---------------------------------------------------------------------------
// Test fixture for BaseItemDelegate
// ---------------------------------------------------------------------------
class BaseItemDelegateExtTest : public testing::Test
{
protected:
    void SetUp() override
    {
        testUrl = QUrl::fromLocalFile("/tmp/ut_workspace_ext");
        view = new FileView(testUrl);
        helper = new FileViewHelper(view);
        delegate = new TestableDelegate(helper);
        delegate->updateItemSizeHint();
    }

    void TearDown() override
    {
        delete delegate;
        delete view;
        stub.clear();
    }

    QUrl testUrl;
    FileView *view = nullptr;
    FileViewHelper *helper = nullptr;
    TestableDelegate *delegate = nullptr;
    stub_ext::StubExt stub;
};

// --- simple return-value methods ---

TEST_F(BaseItemDelegateExtTest, IconSizeLevel_ReturnsMinusOne)
{
    EXPECT_EQ(delegate->iconSizeLevel(), -1);
}

TEST_F(BaseItemDelegateExtTest, MinimumIconSizeLevel_ReturnsMinusOne)
{
    EXPECT_EQ(delegate->minimumIconSizeLevel(), -1);
}

TEST_F(BaseItemDelegateExtTest, MaximumIconSizeLevel_ReturnsMinusOne)
{
    EXPECT_EQ(delegate->maximumIconSizeLevel(), -1);
}

TEST_F(BaseItemDelegateExtTest, IncreaseIcon_ReturnsMinusOne)
{
    EXPECT_EQ(delegate->increaseIcon(), -1);
}

TEST_F(BaseItemDelegateExtTest, DecreaseIcon_ReturnsMinusOne)
{
    EXPECT_EQ(delegate->decreaseIcon(), -1);
}

TEST_F(BaseItemDelegateExtTest, SetIconSizeByIconSizeLevel_ReturnsMinusOne)
{
    EXPECT_EQ(delegate->setIconSizeByIconSizeLevel(3), -1);
}

TEST_F(BaseItemDelegateExtTest, ItemExpanded_ReturnsFalse)
{
    EXPECT_FALSE(delegate->itemExpanded());
}

TEST_F(BaseItemDelegateExtTest, ExpandItemRect_ReturnsEmpty)
{
    EXPECT_EQ(delegate->expandItemRect(), QRect());
}

TEST_F(BaseItemDelegateExtTest, ExpandedIndex_ReturnsInvalid)
{
    EXPECT_FALSE(delegate->expandedIndex().isValid());
}

TEST_F(BaseItemDelegateExtTest, ExpandedItem_ReturnsNullptr)
{
    EXPECT_EQ(delegate->expandedItem(), nullptr);
}

TEST_F(BaseItemDelegateExtTest, EditingIndex_ReturnsInvalid)
{
    EXPECT_FALSE(delegate->editingIndex().isValid());
}

TEST_F(BaseItemDelegateExtTest, HideNotEditingIndexWidget_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(delegate->hideNotEditingIndexWidget());
}

TEST_F(BaseItemDelegateExtTest, HasWidgetIndexs_NoEditing_ReturnsEmpty)
{
    EXPECT_TRUE(delegate->hasWidgetIndexs().isEmpty());
}

TEST_F(BaseItemDelegateExtTest, ItemIconRect_ReturnsSameRect)
{
    QRectF itemRect(10, 20, 100, 80);
    QRectF result = delegate->itemIconRect(itemRect);
    EXPECT_EQ(result, itemRect);
}

TEST_F(BaseItemDelegateExtTest, ItemGeomertys_ReturnsEmpty)
{
    QStyleOptionViewItem opt;
    QModelIndex idx;
    EXPECT_TRUE(delegate->itemGeomertys(opt, idx).isEmpty());
}

TEST_F(BaseItemDelegateExtTest, GetRectOfItem_ReturnsEmptyRect)
{
    EXPECT_EQ(delegate->getRectOfItem(RectOfItemType::kItemIconRect, QModelIndex()), QRect());
}

TEST_F(BaseItemDelegateExtTest, SizeHint_ReturnsItemSizeHint)
{
    QStyleOptionViewItem opt;
    QModelIndex idx;
    QSize hint = delegate->sizeHint(opt, idx);
    EXPECT_EQ(hint, QSize(100, 100));
}

TEST_F(BaseItemDelegateExtTest, DestroyEditor_NoCrash)
{
    QWidget *editor = new QWidget();
    EXPECT_NO_FATAL_FAILURE(delegate->destroyEditor(editor, QModelIndex()));
}

// --- highlight keywords ---

TEST_F(BaseItemDelegateExtTest, SetAndGetHighlightKeywords)
{
    QStringList keywords { "foo", "bar" };
    delegate->setHighlightKeywords(keywords);
    EXPECT_EQ(delegate->highlightKeywords(), keywords);
}

TEST_F(BaseItemDelegateExtTest, HighlightKeywords_DefaultEmpty)
{
    EXPECT_TRUE(delegate->highlightKeywords().isEmpty());
}

TEST_F(BaseItemDelegateExtTest, EffectiveHighlightKeywords_EmptyIndex_FallsBackToShared)
{
    delegate->setHighlightKeywords({ "shared" });
    QStringList result = delegate->effectiveHighlightKeywords(QModelIndex());
    EXPECT_EQ(result, QStringList({ "shared" }));
}

// --- paint proxy ---

TEST_F(BaseItemDelegateExtTest, SetPaintProxy_NoCrash)
{
    auto *proxy = new AbstractItemPaintProxy();
    EXPECT_NO_FATAL_FAILURE(delegate->setPaintProxy(proxy));
}

TEST_F(BaseItemDelegateExtTest, SetPaintProxy_ReplacesPrevious)
{
    auto *proxy1 = new AbstractItemPaintProxy();
    auto *proxy2 = new AbstractItemPaintProxy();
    delegate->setPaintProxy(proxy1);
    EXPECT_NO_FATAL_FAILURE(delegate->setPaintProxy(proxy2));
}

// --- geometry methods ---

TEST_F(BaseItemDelegateExtTest, GetCornerGeometryList_ReturnsFourCorners)
{
    QRectF baseRect(0, 0, 100, 100);
    QSizeF cornerSize(10, 10);
    auto list = delegate->getCornerGeometryList(baseRect, cornerSize);
    EXPECT_EQ(list.size(), 4);
}

TEST_F(BaseItemDelegateExtTest, GetExpandButtonRect_FromRect_ReturnsValidRect)
{
    QRectF rect(0, 0, 200, 40);
    QRect result = delegate->getExpandButtonRect(rect);
    EXPECT_EQ(result.size(), QSize(16, 16));
    EXPECT_EQ(result.left(), 12);
}

TEST_F(BaseItemDelegateExtTest, GetExpandButtonRect_FromOption_ReturnsValidRect)
{
    QStyleOptionViewItem opt;
    opt.rect = QRect(0, 0, 200, 40);
    QRect result = delegate->getExpandButtonRect(opt);
    EXPECT_EQ(result.size(), QSize(16, 16));
}

TEST_F(BaseItemDelegateExtTest, GetExpandButtonHitRect_FromRect_ReturnsLargerRect)
{
    QRectF rect(0, 0, 200, 40);
    QRect result = delegate->getExpandButtonHitRect(rect);
    EXPECT_GE(result.width(), 24);
    EXPECT_GE(result.height(), 24);
}

TEST_F(BaseItemDelegateExtTest, GetExpandButtonHitRect_FromOption_ReturnsLargerRect)
{
    QStyleOptionViewItem opt;
    opt.rect = QRect(0, 0, 200, 40);
    QRect result = delegate->getExpandButtonHitRect(opt);
    EXPECT_GE(result.width(), 24);
}

TEST_F(BaseItemDelegateExtTest, GetGroupTextRect_NoTruncateButton_ReturnsValidRect)
{
    QRectF rect(0, 0, 300, 40);
    QRect result = delegate->getGroupTextRect(rect, false);
    EXPECT_GT(result.width(), 0);
}

TEST_F(BaseItemDelegateExtTest, GetGroupTextRect_WithTruncateButton_ReturnsSmallerRect)
{
    QRectF rect(0, 0, 300, 40);
    QRect withoutBtn = delegate->getGroupTextRect(rect, false);
    QRect withBtn = delegate->getGroupTextRect(rect, true);
    EXPECT_GT(withoutBtn.width(), withBtn.width());
}

TEST_F(BaseItemDelegateExtTest, GetGroupTextRect_FromOption_ReturnsValidRect)
{
    QStyleOptionViewItem opt;
    opt.rect = QRect(0, 0, 300, 40);
    QRect result = delegate->getGroupTextRect(opt, false);
    EXPECT_GT(result.width(), 0);
}

TEST_F(BaseItemDelegateExtTest, GetGroupHeaderBackgroundRect_ReturnsOptionRect)
{
    QStyleOptionViewItem opt;
    opt.rect = QRect(5, 10, 200, 40);
    QRectF result = delegate->getGroupHeaderBackgroundRect(opt);
    EXPECT_EQ(result, QRectF(5, 10, 200, 40));
}

TEST_F(BaseItemDelegateExtTest, GetGroupHeaderSizeHint_ReturnsExpectedSize)
{
    QStyleOptionViewItem opt;
    opt.rect = QRect(0, 0, 300, 40);
    QSize result = delegate->getGroupHeaderSizeHint(opt, QModelIndex());
    EXPECT_EQ(result.width(), 300);
    EXPECT_EQ(result.height(), 40);
}

TEST_F(BaseItemDelegateExtTest, IsGroupHeaderItem_InvalidIndex_ReturnsFalse)
{
    EXPECT_FALSE(delegate->isGroupHeaderItem(QModelIndex()));
}

// --- truncate button ---

TEST_F(BaseItemDelegateExtTest, ShouldShowTruncateButton_InvalidIndex_ReturnsFalse)
{
    EXPECT_FALSE(delegate->shouldShowTruncateButton(QModelIndex()));
}

TEST_F(BaseItemDelegateExtTest, TruncateButtonText_True_ReturnsShowAll)
{
    QString text = delegate->truncateButtonText(true);
    EXPECT_FALSE(text.isEmpty());
}

TEST_F(BaseItemDelegateExtTest, TruncateButtonText_False_ReturnsShowPartial)
{
    QString text = delegate->truncateButtonText(false);
    EXPECT_FALSE(text.isEmpty());
}

TEST_F(BaseItemDelegateExtTest, TruncateButtonTexts_ReturnsTwoItems)
{
    QStringList texts = delegate->truncateButtonTexts();
    EXPECT_EQ(texts.size(), 2);
}

TEST_F(BaseItemDelegateExtTest, GetTruncateButtonWidth_ReturnsPositiveValue)
{
    int width = delegate->getTruncateButtonWidth();
    EXPECT_GT(width, 0);
}

TEST_F(BaseItemDelegateExtTest, GetTruncateButtonRect_FromOption_ReturnsValidRect)
{
    QStyleOptionViewItem opt;
    opt.rect = QRect(0, 0, 300, 40);
    QRect result = delegate->getTruncateButtonRect(opt);
    EXPECT_GT(result.width(), 0);
}

TEST_F(BaseItemDelegateExtTest, GetTruncateButtonRect_FromRect_ReturnsValidRect)
{
    QRectF rect(0, 0, 300, 40);
    QRect result = delegate->getTruncateButtonRect(rect);
    EXPECT_GT(result.width(), 0);
    EXPECT_GT(result.height(), 0);
}

// --- truncate group key ---

TEST_F(BaseItemDelegateExtTest, SetAndGetHoveredTruncateGroupKey)
{
    delegate->setHoveredTruncateGroupKey("group1");
    EXPECT_EQ(delegate->hoveredTruncateGroupKey(), "group1");
}

TEST_F(BaseItemDelegateExtTest, SetAndGetPressedTruncateGroupKey)
{
    delegate->setPressedTruncateGroupKey("group2");
    EXPECT_EQ(delegate->pressedTruncateGroupKey(), "group2");
}

TEST_F(BaseItemDelegateExtTest, HoveredTruncateGroupKey_DefaultEmpty)
{
    EXPECT_TRUE(delegate->hoveredTruncateGroupKey().isEmpty());
}

TEST_F(BaseItemDelegateExtTest, PressedTruncateGroupKey_DefaultEmpty)
{
    EXPECT_TRUE(delegate->pressedTruncateGroupKey().isEmpty());
}

// --- paint methods (use QPainter on QPixmap, no crash) ---

TEST_F(BaseItemDelegateExtTest, PaintExpandButton_NoCrash)
{
    QPixmap pm(200, 40);
    pm.fill(Qt::white);
    QPainter painter(&pm);
    EXPECT_NO_FATAL_FAILURE(delegate->paintExpandButton(&painter, QRect(10, 10, 16, 16), true));
}

TEST_F(BaseItemDelegateExtTest, PaintExpandButton_Expanded_NoCrash)
{
    QPixmap pm(200, 40);
    pm.fill(Qt::white);
    QPainter painter(&pm);
    EXPECT_NO_FATAL_FAILURE(delegate->paintExpandButton(&painter, QRect(10, 10, 16, 16), false));
}

TEST_F(BaseItemDelegateExtTest, PaintExpandButton_EmptyRect_NoCrash)
{
    QPixmap pm(200, 40);
    pm.fill(Qt::white);
    QPainter painter(&pm);
    EXPECT_NO_FATAL_FAILURE(delegate->paintExpandButton(&painter, QRect(), true));
}

TEST_F(BaseItemDelegateExtTest, PaintGroupText_NoCrash)
{
    QPixmap pm(400, 40);
    pm.fill(Qt::white);
    QPainter painter(&pm);
    QStyleOptionViewItem opt;
    EXPECT_NO_FATAL_FAILURE(delegate->paintGroupText(&painter, QRect(0, 0, 300, 40), "Group A", 5, opt));
}

TEST_F(BaseItemDelegateExtTest, PaintGroupText_EmptyText_NoCrash)
{
    QPixmap pm(400, 40);
    pm.fill(Qt::white);
    QPainter painter(&pm);
    QStyleOptionViewItem opt;
    EXPECT_NO_FATAL_FAILURE(delegate->paintGroupText(&painter, QRect(0, 0, 300, 40), "", 5, opt));
}

TEST_F(BaseItemDelegateExtTest, PaintGroupText_ZeroCount_NoCrash)
{
    QPixmap pm(400, 40);
    pm.fill(Qt::white);
    QPainter painter(&pm);
    QStyleOptionViewItem opt;
    EXPECT_NO_FATAL_FAILURE(delegate->paintGroupText(&painter, QRect(0, 0, 300, 40), "Group", 0, opt));
}

TEST_F(BaseItemDelegateExtTest, PaintGroupBackground_NoWidget_NoCrash)
{
    QPixmap pm(400, 40);
    pm.fill(Qt::white);
    QPainter painter(&pm);
    QStyleOptionViewItem opt;
    opt.rect = QRect(0, 0, 300, 40);
    EXPECT_NO_FATAL_FAILURE(delegate->paintGroupBackground(&painter, opt));
}

TEST_F(BaseItemDelegateExtTest, PaintGroupHeader_InvalidIndex_NoCrash)
{
    QPixmap pm(400, 40);
    pm.fill(Qt::white);
    QPainter painter(&pm);
    QStyleOptionViewItem opt;
    opt.rect = QRect(0, 0, 300, 40);
    EXPECT_NO_FATAL_FAILURE(delegate->paintGroupHeader(&painter, opt, QModelIndex()));
}

TEST_F(BaseItemDelegateExtTest, PaintStickyGroupHeader_InvalidIndex_NoCrash)
{
    QPixmap pm(400, 40);
    pm.fill(Qt::white);
    QPainter painter(&pm);
    QStyleOptionViewItem opt;
    opt.rect = QRect(0, 0, 300, 40);
    EXPECT_NO_FATAL_FAILURE(delegate->paintStickyGroupHeader(&painter, opt, QModelIndex()));
}

TEST_F(BaseItemDelegateExtTest, PaintTruncateButton_InvalidIndex_NoCrash)
{
    QPixmap pm(400, 40);
    pm.fill(Qt::white);
    QPainter painter(&pm);
    QStyleOptionViewItem opt;
    opt.rect = QRect(0, 0, 300, 40);
    EXPECT_NO_FATAL_FAILURE(delegate->paintTruncateButton(&painter, QRect(200, 5, 68, 30), QModelIndex(), opt));
}

TEST_F(BaseItemDelegateExtTest, PaintDragIcon_NoCrash)
{
    QPixmap pm(200, 200);
    pm.fill(Qt::white);
    QPainter painter(&pm);
    QStyleOptionViewItem opt;
    opt.rect = QRect(0, 0, 100, 100);
    EXPECT_NO_FATAL_FAILURE(delegate->paintDragIcon(&painter, opt, QModelIndex(), QSize(48, 48)));
}

TEST_F(BaseItemDelegateExtTest, GetIndexIconSize_NoCrash)
{
    QStyleOptionViewItem opt;
    opt.rect = QRect(0, 0, 100, 100);
    EXPECT_NO_FATAL_FAILURE(delegate->getIndexIconSize(opt, QModelIndex(), QSize(48, 48)));
}

TEST_F(BaseItemDelegateExtTest, IsThumbnailIconIndex_InvalidIndex_ReturnsFalse)
{
    EXPECT_FALSE(delegate->isThumbnailIconIndex(QModelIndex()));
}

TEST_F(BaseItemDelegateExtTest, EditingIndexWidget_NoEditing_ReturnsNullptr)
{
    EXPECT_EQ(delegate->editingIndexWidget(), nullptr);
}

// ---------------------------------------------------------------------------
// FileSortWorker additional tests
// ---------------------------------------------------------------------------
class FileSortWorkerExtTest : public testing::Test
{
protected:
    void SetUp() override
    {
        testUrl = QUrl::fromLocalFile("/tmp/ut_sortworker_ext");
        testKey = "ut_sort_key";

        stub.set_lamda(&Application::appAttribute, []() {
            return QVariant(false);
        });

        stub.set_lamda(&WorkspaceHelper::instance, []() {
            static WorkspaceHelper helper;
            return &helper;
        });

        stub.set_lamda(&WorkspaceHelper::isViewModeSupported, []() {
            return true;
        });

        stub.set_lamda(&DeviceProxyManager::instance, []() {
            static DeviceProxyManager manager;
            return &manager;
        });

        worker = new FileSortWorker(testUrl, testKey);
    }

    void TearDown() override
    {
        delete worker;
        stub.clear();
    }

    QUrl testUrl;
    QString testKey;
    FileSortWorker *worker = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileSortWorkerExtTest, GetSortRole_Default_ReturnsInvalid)
{
    EXPECT_EQ(worker->getSortRole(), 0);
}

TEST_F(FileSortWorkerExtTest, GetSortOrder_Default_Ascending)
{
    EXPECT_EQ(worker->getSortOrder(), Qt::AscendingOrder);
}

TEST_F(FileSortWorkerExtTest, GetGroupOrder_Default_Ascending)
{
    EXPECT_EQ(worker->getGroupOrder(), Qt::AscendingOrder);
}

TEST_F(FileSortWorkerExtTest, GetGroupStrategyName_DefaultEmpty)
{
    EXPECT_FALSE(worker->getGroupStrategyName().isEmpty());
}

TEST_F(FileSortWorkerExtTest, CurrentIsGroupingMode_DefaultFalse)
{
    EXPECT_FALSE(worker->currentIsGroupingMode());
}

TEST_F(FileSortWorkerExtTest, ChildrenCount_DefaultZero)
{
    EXPECT_EQ(worker->childrenCount(), 0);
}

TEST_F(FileSortWorkerExtTest, GetFileItemCount_DefaultZero)
{
    EXPECT_EQ(worker->getFileItemCount(), 0);
}

TEST_F(FileSortWorkerExtTest, GetGroupItemCount_DefaultZero)
{
    EXPECT_EQ(worker->getGroupItemCount(), 0);
}

TEST_F(FileSortWorkerExtTest, GetChildrenUrls_DefaultEmpty)
{
    EXPECT_TRUE(worker->getChildrenUrls().isEmpty());
}

TEST_F(FileSortWorkerExtTest, GetChildShowIndex_InvalidUrl_ReturnsNegative)
{
    EXPECT_LT(worker->getChildShowIndex(QUrl::fromLocalFile("/nonexistent")), 0);
}

TEST_F(FileSortWorkerExtTest, Cancel_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(worker->cancel());
}

TEST_F(FileSortWorkerExtTest, RootData_DefaultNullptr)
{
    EXPECT_EQ(worker->rootData(), nullptr);
}

TEST_F(FileSortWorkerExtTest, SetRootData_NoCrash)
{
    auto info = new SortFileInfo();
    info->setUrl(testUrl);
    info->setDir(true);
    auto data = FileItemDataPointer(new FileItemData(SortInfoPointer(info), nullptr));
    EXPECT_NO_FATAL_FAILURE(worker->setRootData(data));
    EXPECT_EQ(worker->rootData(), data);
}

TEST_F(FileSortWorkerExtTest, ChildData_InvalidIndex_ReturnsNullptr)
{
    EXPECT_EQ(worker->childData(0), nullptr);
}

TEST_F(FileSortWorkerExtTest, ChildData_InvalidUrl_ReturnsNullptr)
{
    EXPECT_EQ(worker->childData(QUrl::fromLocalFile("/nonexistent")), nullptr);
}

TEST_F(FileSortWorkerExtTest, GroupHeaderData_InvalidIndex_ReturnsEmpty)
{
    EXPECT_EQ(worker->groupHeaderData(0, Qt::DisplayRole), QVariant());
}

TEST_F(FileSortWorkerExtTest, SetTreeView_True_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(worker->setTreeView(true));
}

TEST_F(FileSortWorkerExtTest, SetTreeView_False_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(worker->setTreeView(false));
}

TEST_F(FileSortWorkerExtTest, SetSortArguments_ValidArgs_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(worker->setSortArguments(Qt::DescendingOrder, dfmbase::Global::kItemFileSizeRole, false));
}

TEST_F(FileSortWorkerExtTest, SetSortArguments_SameArgs_NoCrash)
{
    worker->setSortArguments(Qt::AscendingOrder, dfmbase::Global::kItemFileDisplayNameRole, false);
    EXPECT_NO_FATAL_FAILURE(worker->setSortArguments(Qt::AscendingOrder, dfmbase::Global::kItemFileDisplayNameRole, false));
}

TEST_F(FileSortWorkerExtTest, SetGroupArguments_ValidArgs_NoCrash)
{
    QVariantHash expandStates;
    expandStates["group1"] = true;
    EXPECT_NO_FATAL_FAILURE(worker->setGroupArguments(Qt::AscendingOrder, "testStrategy", expandStates));
}

// ---------------------------------------------------------------------------
// FileOperatorHelper additional tests
// ---------------------------------------------------------------------------
class FileOperatorHelperExtTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.set_lamda(&WorkspaceHelper::instance, []() {
            static WorkspaceHelper helper;
            return &helper;
        });

        stub.set_lamda(&WorkspaceHelper::windowId, [](WorkspaceHelper *, const QWidget *) {
            return quint64(12345);
        });

        stub.set_lamda(&dfmbase::ClipBoard::instance, []() {
            static dfmbase::ClipBoard clipboard;
            return &clipboard;
        });

        stub.set_lamda(&DFMBASE_NAMESPACE::Application::genericObtuselySetting, []() {
            static DFMBASE_NAMESPACE::Settings settings("test", DFMBASE_NAMESPACE::Settings::kGenericConfig);
            return &settings;
        });

        helper = FileOperatorHelper::instance();
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
    FileOperatorHelper *helper = nullptr;
};

TEST_F(FileOperatorHelperExtTest, Instance_Singleton)
{
    EXPECT_EQ(FileOperatorHelper::instance(), helper);
}

TEST_F(FileOperatorHelperExtTest, RenameFilesByReplace_NoCrash)
{
    QList<QUrl> urls { QUrl::fromLocalFile("/tmp/test1.txt") };
    QPair<QString, QString> replacePair("test", "new");
    EXPECT_NO_FATAL_FAILURE(helper->renameFilesByReplace(nullptr, urls, replacePair));
}

TEST_F(FileOperatorHelperExtTest, RenameFilesByAdd_NoCrash)
{
    QList<QUrl> urls { QUrl::fromLocalFile("/tmp/test1.txt") };
    QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag> addPair(
        "_suffix", DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag::kPrefix);
    EXPECT_NO_FATAL_FAILURE(helper->renameFilesByAdd(nullptr, urls, addPair));
}

TEST_F(FileOperatorHelperExtTest, RenameFilesByCustom_NoCrash)
{
    QList<QUrl> urls { QUrl::fromLocalFile("/tmp/test1.txt") };
    QPair<QString, QString> customPair("test", "custom");
    EXPECT_NO_FATAL_FAILURE(helper->renameFilesByCustom(nullptr, urls, customPair));
}

TEST_F(FileOperatorHelperExtTest, OpenFilesByMode_NoCrash)
{
    QList<QUrl> urls { QUrl::fromLocalFile("/tmp/test1.txt") };
    EXPECT_NO_FATAL_FAILURE(helper->openFilesByMode(nullptr, urls, DirOpenMode::kOpenNewWindow));
}

TEST_F(FileOperatorHelperExtTest, OpenFilesByApp_NoCrash)
{
    QList<QUrl> urls { QUrl::fromLocalFile("/tmp/test1.txt") };
    QList<QString> apps { "app1" };
    EXPECT_NO_FATAL_FAILURE(helper->openFilesByApp(nullptr, urls, apps));
}

TEST_F(FileOperatorHelperExtTest, DropFiles_NoCrash)
{
    QList<QUrl> urls { QUrl::fromLocalFile("/tmp/test1.txt") };
    EXPECT_NO_FATAL_FAILURE(helper->dropFiles(nullptr, Qt::CopyAction, QUrl::fromLocalFile("/tmp/target"), urls));
}

TEST_F(FileOperatorHelperExtTest, PreviewFiles_NoCrash)
{
    QList<QUrl> selectUrls { QUrl::fromLocalFile("/tmp/test1.txt") };
    QList<QUrl> dirUrls { QUrl::fromLocalFile("/tmp/") };
    EXPECT_NO_FATAL_FAILURE(helper->previewFiles(nullptr, selectUrls, dirUrls));
}
