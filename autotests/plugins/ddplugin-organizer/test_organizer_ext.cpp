// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "view/collectionview.h"
#include "view/collectionview_p.h"
#include "delegate/collectionitemdelegate.h"
#include "delegate/collectionitemdelegate_p.h"
#include "models/collectionmodel.h"
#include "mode/collectiondataprovider.h"
#include "mode/normalizedmode.h"
#include "mode/normalized/normalizedmode_p.h"
#include "framemanager.h"
#include "private/framemanager_p.h"
#include "interface/canvasmodelshell.h"
#include "interface/canvasviewshell.h"
#include "interface/canvasgridshell.h"
#include "interface/canvasmanagershell.h"

#include <dfm-base/utils/elidetextlayout.h>

#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QModelIndex>
#include <QRect>
#include <QPoint>
#include <QMargins>
#include <QStyleOptionViewItem>
#include <QItemSelectionModel>
#include <QStandardItemModel>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMessage>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_organizer;

class MockCollectionDataProvider2 : public CollectionDataProvider
{
public:
    MockCollectionDataProvider2() : CollectionDataProvider(nullptr) {}
    QString replace(const QUrl &, const QUrl &) override { return QString(); }
    QString append(const QUrl &) override { return QString(); }
    QString prepend(const QUrl &) override { return QString(); }
    void insert(const QUrl &, const QString &, const int) override {}
    QString remove(const QUrl &) override { return QString(); }
    QString change(const QUrl &) override { return QString(); }
};

class UT_OrganizerExt : public testing::Test
{
protected:
    void SetUp() override
    {
        provider = new MockCollectionDataProvider2();
        view = new CollectionView("test_uuid", provider);
        model = new CollectionModel();
        view->setModel(model);

        stub.set_lamda(&QWidget::show, [](QWidget *) { __DBG_STUB_INVOKE__ });
        stub.set_lamda(&QWidget::hide, [](QWidget *) { __DBG_STUB_INVOKE__ });
    }

    void TearDown() override
    {
        delete view;
        delete provider;
        delete model;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    CollectionView *view = nullptr;
    MockCollectionDataProvider2 *provider = nullptr;
    CollectionModel *model = nullptr;
};

// ============ CollectionViewPrivate ============

TEST_F(UT_OrganizerExt, Private_PointToPos)
{
    view->d->viewMargins = QMargins(10, 10, 10, 10);
    view->d->cellWidth = 100;
    view->d->cellHeight = 100;
    QPoint result = view->d->pointToPos(QPoint(110, 210));
    EXPECT_EQ(result.x(), 1);
    EXPECT_EQ(result.y(), 2);
}

TEST_F(UT_OrganizerExt, Private_PosToNode)
{
    view->d->columnCount = 5;
    view->d->rowCount = 5;
    EXPECT_EQ(view->d->posToNode(QPoint(2, 1)), 7);
    EXPECT_EQ(view->d->posToNode(QPoint(0, 0)), 0);
}

TEST_F(UT_OrganizerExt, Private_PosToPoint)
{
    view->d->cellWidth = 50;
    view->d->cellHeight = 60;
    view->d->viewMargins = QMargins(5, 5, 5, 5);
    QPoint result = view->d->posToPoint(QPoint(2, 3));
    EXPECT_EQ(result.x(), 105);
    EXPECT_EQ(result.y(), 185);
}

TEST_F(UT_OrganizerExt, Private_NodeToPos)
{
    view->d->columnCount = 4;
    EXPECT_EQ(view->d->nodeToPos(7).x(), 3);
    EXPECT_EQ(view->d->nodeToPos(7).y(), 1);
    EXPECT_EQ(view->d->nodeToPos(0).x(), 0);
    EXPECT_EQ(view->d->nodeToPos(0).y(), 0);
}

TEST_F(UT_OrganizerExt, Private_IsDelayDrag)
{
    EXPECT_FALSE(view->d->isDelayDrag());
    view->d->touchDragTimer.start(1000);
    EXPECT_TRUE(view->d->isDelayDrag());
    view->d->touchDragTimer.stop();
    EXPECT_FALSE(view->d->isDelayDrag());
}

TEST_F(UT_OrganizerExt, Private_VisualRect)
{
    view->d->cellWidth = 80;
    view->d->cellHeight = 90;
    view->d->viewMargins = QMargins(0, 0, 0, 0);
    QRect rect = view->d->visualRect(QPoint(1, 2));
    EXPECT_EQ(rect.x(), 80);
    EXPECT_EQ(rect.y(), 180);
    EXPECT_EQ(rect.width(), 80);
    EXPECT_EQ(rect.height(), 90);
}

TEST_F(UT_OrganizerExt, Private_UpdateCellMargins)
{
    view->d->updateCellMargins(QSize(40, 50), QSize(100, 100));
    EXPECT_EQ(view->d->cellMargins.left(), 30);
    EXPECT_EQ(view->d->cellMargins.top(), 25);
}

TEST_F(UT_OrganizerExt, Private_UpdateColumnCount)
{
    view->d->viewMargins = QMargins(10, 0, 10, 0);
    view->d->updateColumnCount(320, 50);
    EXPECT_EQ(view->d->columnCount, 6);
    EXPECT_EQ(view->d->cellWidth, 50);
}

TEST_F(UT_OrganizerExt, Private_UpdateColumnCount_TooSmall)
{
    view->d->viewMargins = QMargins(10, 0, 10, 0);
    view->d->updateColumnCount(5, 50);
    EXPECT_EQ(view->d->columnCount, 1);
    EXPECT_EQ(view->d->cellWidth, 5);
}

TEST_F(UT_OrganizerExt, Private_UpdateRowCount)
{
    view->d->viewMargins = QMargins(0, 10, 0, 10);
    view->d->updateRowCount(320, 50);
    EXPECT_EQ(view->d->rowCount, 6);
}

TEST_F(UT_OrganizerExt, Private_UpdateRowCount_TooSmall)
{
    view->d->viewMargins = QMargins(0, 10, 0, 10);
    view->d->updateRowCount(5, 50);
    EXPECT_EQ(view->d->rowCount, 1);
}

TEST_F(UT_OrganizerExt, Private_UpdateViewMargins)
{
    QMargins testMargins(5, 10, 15, 20);
    view->d->updateViewMargins(QSize(100, 100), testMargins);
    EXPECT_EQ(view->d->viewMargins, testMargins);
}

TEST_F(UT_OrganizerExt, Private_UpdateViewSizeData)
{
    QMargins margins(10, 10, 10, 10);
    view->d->updateViewSizeData(QSize(320, 320), margins, QSize(50, 50));
    EXPECT_GT(view->d->columnCount, 0);
    EXPECT_GT(view->d->rowCount, 0);
    EXPECT_GT(view->d->cellWidth, 0);
    EXPECT_GT(view->d->cellHeight, 0);
}

TEST_F(UT_OrganizerExt, Private_UpdateTarget)
{
    QMimeData mimeData;
    QUrl url("file:///test");
    view->d->updateTarget(&mimeData, url);
    EXPECT_EQ(view->d->dropTargetUrl, url);
}

TEST_F(UT_OrganizerExt, Private_DrawDragText)
{
    QPixmap pixmap(200, 100);
    pixmap.fill(Qt::black);
    QPainter painter(&pixmap);
    EXPECT_NO_FATAL_FAILURE(view->d->drawDragText(&painter, "test", QRect(0, 0, 200, 100)));
    painter.end();
}

TEST_F(UT_OrganizerExt, Private_DrawEllipseBackground)
{
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    EXPECT_NO_FATAL_FAILURE(view->d->drawEllipseBackground(&painter, QRect(10, 10, 80, 80)));
    painter.end();
}

TEST_F(UT_OrganizerExt, Private_OnItemsChanged_DifferentKey)
{
    EXPECT_NO_FATAL_FAILURE(view->d->onItemsChanged("other_uuid"));
}

TEST_F(UT_OrganizerExt, Private_OnItemsChanged_SameKey)
{
    stub.set_lamda(&CollectionViewPrivate::updateVerticalBarRange, []() { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(view->d->onItemsChanged("test_uuid"));
}

TEST_F(UT_OrganizerExt, Private_ItemPaintGeomertys_InvalidIndex)
{
    QList<QRect> result = view->d->itemPaintGeomertys(QModelIndex());
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_OrganizerExt, Private_VerticalScrollToValue)
{
    QModelIndex idx;
    QRect rect(0, 0, 50, 50);
    int result = view->d->verticalScrollToValue(idx, rect, QAbstractItemView::EnsureVisible);
    EXPECT_GE(result, 0);
}

// ============ GraphicsEffect ============

TEST_F(UT_OrganizerExt, GraphicsEffect_BoundingRectFor)
{
    GraphicsEffect effect(view);
    QRectF source(10, 20, 100, 200);
    QRectF result = effect.boundingRectFor(source);
    EXPECT_EQ(result, source);
}

TEST_F(UT_OrganizerExt, GraphicsEffect_SourceChanged)
{
    GraphicsEffect effect(view);
    EXPECT_NO_FATAL_FAILURE(effect.sourceChanged(QGraphicsEffect::SourceAttached));
}

// ============ CollectionItemDelegate ============

class UT_DelegateExt : public testing::Test
{
protected:
    void SetUp() override
    {
        provider = new MockCollectionDataProvider2();
        view = new CollectionView("test_uuid", provider);
        delegate = new CollectionItemDelegate(view);
        stub.set_lamda(&QWidget::show, [](QWidget *) { __DBG_STUB_INVOKE__ });
        stub.set_lamda(&QWidget::hide, [](QWidget *) { __DBG_STUB_INVOKE__ });
    }

    void TearDown() override
    {
        delete delegate;
        delete view;
        delete provider;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    CollectionView *view = nullptr;
    MockCollectionDataProvider2 *provider = nullptr;
    CollectionItemDelegate *delegate = nullptr;
};


TEST_F(UT_DelegateExt, ElideTextRect)
{
    auto result = delegate->elideTextRect(QModelIndex(), QRect(0, 0, 100, 20), Qt::ElideRight);
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(UT_DelegateExt, TextPaintRect)
{
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);
    QRect result = delegate->textPaintRect(option, QModelIndex(), QRect(0, 0, 100, 100), false);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_DelegateExt, Paint_Background)
{
    QPixmap pixmap(200, 200);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);
    EXPECT_NO_FATAL_FAILURE(delegate->paintBackground(&painter, option, QRect(0, 0, 100, 100)));
    painter.end();
}

TEST_F(UT_DelegateExt, Paint_Emblems)
{
    QPixmap pixmap(200, 200);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);
    EXPECT_NO_FATAL_FAILURE(delegate->paintEmblems(&painter, QRectF(0, 0, 100, 20), nullptr));
    painter.end();
}

TEST_F(UT_DelegateExt, Paint_Label)
{
    QPixmap pixmap(200, 200);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);
    EXPECT_NO_FATAL_FAILURE(delegate->paintLabel(&painter, option, QModelIndex(), QRect(0, 0, 100, 20)));
    painter.end();
}


TEST_F(UT_DelegateExt, DrawNormlText)
{
    QPixmap pixmap(200, 200);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);
    EXPECT_NO_FATAL_FAILURE(delegate->drawNormlText(&painter, option, QModelIndex(), QRectF(0, 0, 100, 20)));
    painter.end();
}

TEST_F(UT_DelegateExt, DrawHighlightText)
{
    QPixmap pixmap(200, 200);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);
    EXPECT_NO_FATAL_FAILURE(delegate->drawHighlightText(&painter, option, QModelIndex(), QRect(0, 0, 100, 20)));
    painter.end();
}

TEST_F(UT_DelegateExt, DrawExpandText)
{
    QPixmap pixmap(200, 200);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);
    EXPECT_NO_FATAL_FAILURE(delegate->drawExpandText(&painter, option, QModelIndex(), QRectF(0, 0, 100, 20)));
    painter.end();
}

TEST_F(UT_DelegateExt, SetEditorData)
{
    EXPECT_NO_FATAL_FAILURE(delegate->setEditorData(nullptr, QModelIndex()));
}

TEST_F(UT_DelegateExt, UpdateEditorGeometry)
{
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 30);
    EXPECT_NO_FATAL_FAILURE(delegate->updateEditorGeometry(nullptr, option, QModelIndex()));
}


// ============ NormalizedMode ============

class UT_NormalizedExt : public testing::Test
{
protected:
    void SetUp() override
    {
        mode = new NormalizedMode();
        stub.set_lamda(&QWidget::show, [](QWidget *) { __DBG_STUB_INVOKE__ });
        stub.set_lamda(&QWidget::hide, [](QWidget *) { __DBG_STUB_INVOKE__ });
    }

    void TearDown() override
    {
        delete mode;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    NormalizedMode *mode = nullptr;
};

TEST_F(UT_NormalizedExt, FilterContextMenu)
{
    bool result = mode->filterContextMenu(0, QUrl(), QList<QUrl>(), QPoint());
    EXPECT_FALSE(result);
}

TEST_F(UT_NormalizedExt, DetachLayout_Empty)
{
    EXPECT_NO_FATAL_FAILURE(mode->detachLayout());
}

TEST_F(UT_NormalizedExt, FilterDataInserted_NoClassifier)
{
    QUrl url("file:///test");
    bool result = mode->filterDataInserted(url);
    EXPECT_FALSE(result);
}

TEST_F(UT_NormalizedExt, NormalizedPrivate_OnDropFile)
{
    QList<QUrl> urls;
    urls << QUrl("file:///test1") << QUrl("file:///test2");
    EXPECT_NO_FATAL_FAILURE(mode->d->onDropFile("collection1", urls));
    EXPECT_TRUE(urls.isEmpty());
}

TEST_F(UT_NormalizedExt, NormalizedPrivate_GenerateScreenConfigId)
{
    QString id = mode->d->generateScreenConfigId();
    EXPECT_TRUE(id.startsWith("StyleConfig"));
}

TEST_F(UT_NormalizedExt, NormalizedPrivate_RefreshViews_Empty)
{
    EXPECT_NO_FATAL_FAILURE(mode->d->refreshViews(true));
    EXPECT_NO_FATAL_FAILURE(mode->d->refreshViews(false));
}

TEST_F(UT_NormalizedExt, NormalizedPrivate_OnFontChanged_Empty)
{
    EXPECT_NO_FATAL_FAILURE(mode->d->onFontChanged());
}


TEST_F(UT_NormalizedExt, NormalizedPrivate_CollectionStyleChanged_Empty)
{
    EXPECT_NO_FATAL_FAILURE(mode->d->collectionStyleChanged("nonexistent"));
}

// ============ FrameManagerPrivate ============

class UT_FrameManagerExt : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.set_lamda(ADDR(QDBusInterface, doCall), []() {
            __DBG_STUB_INVOKE__
            return QDBusMessage();
        });
        manager = new FrameManager();
    }

    void TearDown() override
    {
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    FrameManager *manager = nullptr;
};

TEST_F(UT_FrameManagerExt, Private_EnableVisibility)
{
    EXPECT_NO_FATAL_FAILURE(manager->d->enableVisibility(true));
    EXPECT_NO_FATAL_FAILURE(manager->d->enableVisibility(false));
}

TEST_F(UT_FrameManagerExt, Private_FindView_NullRoot)
{
    EXPECT_EQ(manager->d->findView(nullptr), nullptr);
}

TEST_F(UT_FrameManagerExt, Private_FindView_NoCanvas)
{
    QWidget root;
    QWidget child(&root);
    child.setProperty("organizersurface", "notcanvas");
    EXPECT_EQ(manager->d->findView(&root), nullptr);
}

TEST_F(UT_FrameManagerExt, Private_OnNotificationClosed_DifferentId)
{
    EXPECT_NO_FATAL_FAILURE(manager->d->onNotificationClosed(999, 0));
}

TEST_F(UT_FrameManagerExt, Private_RefeshCanvas_NullCanvas)
{
    EXPECT_NO_FATAL_FAILURE(manager->d->refeshCanvas());
}

TEST_F(UT_FrameManagerExt, Private_CreateSurface_NullRoot)
{
    auto surface = manager->d->createSurface(nullptr);
    EXPECT_EQ(surface, nullptr);
}

TEST_F(UT_FrameManagerExt, Private_CreateSurface_Valid)
{
    QWidget root;
    root.setProperty("ScreenName", "screen1");
    auto surface = manager->d->createSurface(&root);
    EXPECT_NE(surface, nullptr);
    EXPECT_EQ(surface->property("ScreenName").toString(), "screen1");
}

TEST_F(UT_FrameManagerExt, OrganizerEnabled)
{
    bool result = manager->organizerEnabled();
    EXPECT_FALSE(result);
}
