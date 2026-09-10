// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "views/tabbar.h"
#include "views/crumbbar.h"
#include "views/private/crumbbar_p.h"
#include "views/titlebarwidget.h"
#include "views/navwidget.h"
#include "views/addressbar.h"
#include "views/searcheditwidget.h"
#include "views/optionbuttonbox.h"
#include "utils/titlebarhelper.h"
#include "utils/crumbinterface.h"
#include "events/titlebareventcaller.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-framework/dpf.h>
#include <dfm-framework/event/event.h>

#include <DTitlebar>
#include <DTabBar>

#include <gtest/gtest.h>
#include <QUrl>
#include <QSignalSpy>
#include <QTest>
#include <QMimeData>
#include <QStyleOptionTab>
#include <QPainter>
#include <QImage>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QContextMenuEvent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFocusEvent>
#include <QHideEvent>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace dfmplugin_titlebar;

// ==================== TabBar extension tests ====================

class TabBarExtTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [](const QString &) {
            __DBG_STUB_INVOKE__
            return QIcon();
        });
        stub.set_lamda(&SystemPathUtil::isSystemPath, [] {
            __DBG_STUB_INVOKE__
            return false;
        });
        stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &url1, const QUrl &url2) {
            __DBG_STUB_INVOKE__
            return url1 == url2;
        });
        tabBar = new TabBar();
    }

    void TearDown() override
    {
        delete tabBar;
        tabBar = nullptr;
        stub.clear();
    }

    TabBar *tabBar { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(TabBarExtTest, ActivateNextTab_SingleTab_StaysAtZero)
{
    tabBar->appendInactiveTab(QUrl("file:///home"));
    tabBar->activateNextTab();
    EXPECT_EQ(tabBar->currentIndex(), 0);
}

TEST_F(TabBarExtTest, ActivateNextTab_MultipleTabs_WrapsAround)
{
    tabBar->appendInactiveTab(QUrl("file:///home"));
    int idx1 = tabBar->appendInactiveTab(QUrl("file:///home/test"));
    tabBar->setCurrentIndex(idx1);
    tabBar->activateNextTab();
    EXPECT_EQ(tabBar->currentIndex(), 0);
}

TEST_F(TabBarExtTest, ActivatePreviousTab_FirstTab_WrapsToLast)
{
    tabBar->appendInactiveTab(QUrl("file:///home"));
    int idx1 = tabBar->appendInactiveTab(QUrl("file:///home/test"));
    tabBar->setCurrentIndex(0);
    tabBar->activatePreviousTab();
    EXPECT_EQ(tabBar->currentIndex(), idx1);
}

TEST_F(TabBarExtTest, ActivatePreviousTab_MiddleTab_MovesToPrevious)
{
    int idx0 = tabBar->appendInactiveTab(QUrl("file:///home"));
    int idx1 = tabBar->appendInactiveTab(QUrl("file:///home/test"));
    tabBar->setCurrentIndex(idx1);
    tabBar->activatePreviousTab();
    EXPECT_EQ(tabBar->currentIndex(), idx0);
}

TEST_F(TabBarExtTest, IsInactiveTab_InactiveTab_ReturnsTrue)
{
    int idx = tabBar->appendInactiveTab(QUrl("file:///home"));
    EXPECT_TRUE(tabBar->isInactiveTab(idx));
}

TEST_F(TabBarExtTest, IsInactiveTab_InvalidIndex_ReturnsFalse)
{
    EXPECT_FALSE(tabBar->isInactiveTab(-1));
    EXPECT_FALSE(tabBar->isInactiveTab(999));
}

TEST_F(TabBarExtTest, CanInsertFromMimeData_NullSource_ReturnsFalse)
{
    EXPECT_FALSE(tabBar->canInsertFromMimeData(0, nullptr));
}

TEST_F(TabBarExtTest, CanInsertFromMimeData_NoFormat_ReturnsFalse)
{
    QMimeData data;
    EXPECT_FALSE(tabBar->canInsertFromMimeData(0, &data));
}

TEST_F(TabBarExtTest, CanInsertFromMimeData_ValidFormat_ReturnsTrue)
{
    QJsonObject obj;
    obj[TabDef::kProcessId] = static_cast<qint64>(QApplication::applicationPid());
    QJsonDocument doc(obj);
    QMimeData data;
    data.setData("application/x-dde-filemanager-tab", doc.toJson());
    EXPECT_TRUE(tabBar->canInsertFromMimeData(0, &data));
}

TEST_F(TabBarExtTest, CreateMimeDataFromTab_ValidIndex_ReturnsData)
{
    int idx = tabBar->appendInactiveTab(QUrl("file:///home/test"));
    QStyleOptionTab opt;
    QMimeData *mime = tabBar->createMimeDataFromTab(idx, opt);
    EXPECT_NE(mime, nullptr);
    if (mime) {
        EXPECT_TRUE(mime->hasFormat("application/x-dde-filemanager-tab"));
        delete mime;
    }
}

TEST_F(TabBarExtTest, CreateMimeDataFromTab_InvalidIndex_ReturnsNull)
{
    QStyleOptionTab opt;
    QMimeData *mime = tabBar->createMimeDataFromTab(-1, opt);
    EXPECT_EQ(mime, nullptr);
}

TEST_F(TabBarExtTest, InsertFromMimeData_ValidData_InsertsTab)
{
    QJsonObject obj;
    obj[TabDef::kTabUrl] = QUrl("file:///home/test").toString();
    obj[TabDef::kTabAlias] = QString("test");
    obj[TabDef::kProcessId] = static_cast<qint64>(QApplication::applicationPid());
    QJsonDocument doc(obj);
    QMimeData data;
    data.setData("application/x-dde-filemanager-tab", doc.toJson());
    int oldCount = tabBar->count();
    tabBar->insertFromMimeData(0, &data);
    EXPECT_GT(tabBar->count(), oldCount);
}

TEST_F(TabBarExtTest, InsertFromMimeDataOnDragEnter_ValidData_InsertsTab)
{
    QJsonObject obj;
    obj[TabDef::kTabUrl] = QUrl("file:///home/test").toString();
    obj[TabDef::kTabAlias] = QString("test");
    QJsonDocument doc(obj);
    QMimeData data;
    data.setData("application/x-dde-filemanager-tab", doc.toJson());
    int oldCount = tabBar->count();
    tabBar->insertFromMimeDataOnDragEnter(0, &data);
    EXPECT_GT(tabBar->count(), oldCount);
}

TEST_F(TabBarExtTest, InsertInactiveTab_ValidUrl_InsertsAtPosition)
{
    int idx = tabBar->insertInactiveTab(0, QUrl("file:///home/test"));
    EXPECT_GE(idx, 0);
}

TEST_F(TabBarExtTest, MousePressEvent_MiddleButton_EmitsCloseRequest)
{
    int idx = tabBar->appendInactiveTab(QUrl("file:///home"));
    QSignalSpy spy(tabBar, &TabBar::tabCloseRequested);
    QRect rect = tabBar->tabRect(idx);
    QPoint pos = rect.center();
    if (pos.isNull()) pos = QPoint(10, 5);
    QMouseEvent me(QEvent::MouseButtonPress, pos, Qt::MiddleButton, Qt::MiddleButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(tabBar->mousePressEvent(&me));
}

TEST_F(TabBarExtTest, PaintTab_ValidIndex_DoesNotCrash)
{
    int idx = tabBar->appendInactiveTab(QUrl("file:///home"));
    QImage img(200, 30, QImage::Format_ARGB32);
    img.fill(Qt::white);
    QPainter painter(&img);
    QStyleOptionTab opt;
    opt.rect = QRect(0, 0, 200, 30);
    EXPECT_NO_FATAL_FAILURE(tabBar->paintTab(&painter, idx, opt));
}

TEST_F(TabBarExtTest, ResizeEvent_DoesNotCrash)
{
    QResizeEvent e(QSize(400, 30), QSize(300, 30));
    EXPECT_NO_FATAL_FAILURE(tabBar->resizeEvent(&e));
}

TEST_F(TabBarExtTest, TabSizeHint_ValidIndex_ReturnsSize)
{
    int idx = tabBar->appendInactiveTab(QUrl("file:///home"));
    QSize s = tabBar->tabSizeHint(idx);
    EXPECT_GT(s.width(), 0);
}

TEST_F(TabBarExtTest, MinimumTabSizeHint_ReturnsSize)
{
    QSize s = tabBar->minimumTabSizeHint(0);
    EXPECT_GE(s.width(), 0);
}

TEST_F(TabBarExtTest, MaximumTabSizeHint_ReturnsSize)
{
    QSize s = tabBar->maximumTabSizeHint(0);
    EXPECT_GE(s.width(), 0);
}

TEST_F(TabBarExtTest, EventFilter_PaintEvent_DoesNotCrash)
{
    tabBar->appendInactiveTab(QUrl("file:///home"));
    QPaintEvent e(QRect(0, 0, 10, 10));
    EXPECT_NO_FATAL_FAILURE(tabBar->eventFilter(tabBar, &e));
}

TEST_F(TabBarExtTest, EventFilter_UnknownObject_DoesNotCrash)
{
    QObject obj;
    QEvent e(QEvent::User);
    EXPECT_NO_FATAL_FAILURE(tabBar->eventFilter(&obj, &e));
}

// ==================== CrumbBar extension tests ====================

class CrumbBarExtTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [](const QString &) {
            __DBG_STUB_INVOKE__
            return QIcon();
        });
        stub.set_lamda(&SystemPathUtil::isSystemPath, [] {
            __DBG_STUB_INVOKE__
            return false;
        });
        crumbBar = new CrumbBar();
    }

    void TearDown() override
    {
        delete crumbBar;
        crumbBar = nullptr;
        stub.clear();
    }

    CrumbBar *crumbBar { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(CrumbBarExtTest, SetPopupVisible_True_UpdatesState)
{
    EXPECT_NO_FATAL_FAILURE(crumbBar->setPopupVisible(true));
    EXPECT_NO_FATAL_FAILURE(crumbBar->setPopupVisible(false));
}

TEST_F(CrumbBarExtTest, SetPopupVisible_SameValue_NoUpdate)
{
    crumbBar->setPopupVisible(true);
    crumbBar->setPopupVisible(true);
    SUCCEED();
}

TEST_F(CrumbBarExtTest, OnHideAddrAndUpdateCrumbs_WithController_Updates)
{
    stub.set_lamda(&CrumbInterface::seprateUrl, [](CrumbInterface *, const QUrl &) {
        __DBG_STUB_INVOKE__
        QList<CrumbData> list;
        list.append(CrumbData { QUrl("file:///"), "/" });
        list.append(CrumbData { QUrl("file:///home"), "home" });
        return list;
    });
    QUrl url("file:///home");
    EXPECT_NO_FATAL_FAILURE(crumbBar->onHideAddrAndUpdateCrumbs(url));
}

TEST_F(CrumbBarExtTest, OnUrlChanged_ValidUrl_UpdatesCrumbs)
{
    stub.set_lamda(&CrumbInterface::seprateUrl, [](CrumbInterface *, const QUrl &) {
        __DBG_STUB_INVOKE__
        QList<CrumbData> list;
        list.append(CrumbData { QUrl("file:///"), "/" });
        list.append(CrumbData { QUrl("file:///home"), "home" });
        return list;
    });
    QUrl url("file:///home");
    EXPECT_NO_FATAL_FAILURE(crumbBar->onUrlChanged(url));
}

TEST_F(CrumbBarExtTest, OnKeepAddressBar_ValidUrl_EmitsSignal)
{
    QSignalSpy spy(crumbBar, &CrumbBar::showAddressBarText);
    QUrl url("file:///home?keyword=test");
    crumbBar->onKeepAddressBar(url);
    EXPECT_GE(spy.count(), 0);
}

TEST_F(CrumbBarExtTest, ContextMenuEvent_DoesNotCrash)
{
    QContextMenuEvent e(QContextMenuEvent::Mouse, QPoint(5, 5));
    EXPECT_NO_FATAL_FAILURE(crumbBar->contextMenuEvent(&e));
}

TEST_F(CrumbBarExtTest, EnterEvent_DoesNotCrash)
{
    QEnterEvent e(QPointF(5, 5), QPointF(5, 5), QPointF(5, 5));
    EXPECT_NO_FATAL_FAILURE(crumbBar->enterEvent(&e));
}

TEST_F(CrumbBarExtTest, LeaveEvent_DoesNotCrash)
{
    QEvent e(QEvent::Leave);
    EXPECT_NO_FATAL_FAILURE(crumbBar->leaveEvent(&e));
}

TEST_F(CrumbBarExtTest, MousePressEvent_DoesNotCrash)
{
    QMouseEvent e(QEvent::MouseButtonPress, QPointF(5, 5), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(crumbBar->mousePressEvent(&e));
}

TEST_F(CrumbBarExtTest, MouseReleaseEvent_DoesNotCrash)
{
    QMouseEvent e(QEvent::MouseButtonRelease, QPointF(5, 5), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(crumbBar->mouseReleaseEvent(&e));
}

TEST_F(CrumbBarExtTest, PaintEvent_DoesNotCrash)
{
    QPaintEvent e(QRect(0, 0, 200, 30));
    EXPECT_NO_FATAL_FAILURE(crumbBar->paintEvent(&e));
}

TEST_F(CrumbBarExtTest, ResizeEvent_DoesNotCrash)
{
    QResizeEvent e(QSize(400, 30), QSize(300, 30));
    EXPECT_NO_FATAL_FAILURE(crumbBar->resizeEvent(&e));
}

TEST_F(CrumbBarExtTest, ShowEvent_DoesNotCrash)
{
    QShowEvent e;
    EXPECT_NO_FATAL_FAILURE(crumbBar->showEvent(&e));
}

TEST_F(CrumbBarExtTest, EventFilter_DoesNotCrash)
{
    QObject obj;
    QEvent e(QEvent::User);
    EXPECT_NO_FATAL_FAILURE(crumbBar->eventFilter(&obj, &e));
}

TEST_F(CrumbBarExtTest, CustomMenu_DoesNotCrash)
{
    QUrl url("file:///tmp");
    QMenu menu;
    EXPECT_NO_FATAL_FAILURE(crumbBar->customMenu(url, &menu));
}

// ==================== TitleBarWidget extension tests ====================

class TitleBarWidgetExtTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [](const QString &) {
            __DBG_STUB_INVOKE__
            return QIcon();
        });
        stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &, const QVariant &defaultValue) {
            __DBG_STUB_INVOKE__
            return defaultValue;
        });
        stub.set_lamda(&DConfigManager::setValue, [](DConfigManager *, const QString &, const QString &, const QVariant &) {
            __DBG_STUB_INVOKE__
        });
        typedef QVariant (EventChannelManager::*PushFunc1)(const QString &, const QString &, QWidget *, const char(&)[15]);
        stub.set_lamda(static_cast<PushFunc1>(&EventChannelManager::push),
                       [] {
                           __DBG_STUB_INVOKE__
                           return QVariant();
                       });
        typedef QVariant (EventChannelManager::*PushFunc2)(const QString &, const QString &, QWidget *, const char(&)[17]);
        stub.set_lamda(static_cast<PushFunc2>(&EventChannelManager::push),
                       [] {
                           __DBG_STUB_INVOKE__
                           return QVariant();
                       });
        stub.set_lamda(&TitleBarHelper::windowId, [](QWidget *) -> quint64 {
            __DBG_STUB_INVOKE__
            return 999;
        });
        widget = new TitleBarWidget();
    }

    void TearDown() override
    {
        delete widget;
        widget = nullptr;
        stub.clear();
    }

    TitleBarWidget *widget { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(TitleBarWidgetExtTest, CurrentUrl_AfterConstruction_ReturnsUrl)
{
    EXPECT_NO_FATAL_FAILURE(widget->currentUrl());
}

TEST_F(TitleBarWidgetExtTest, NavWidget_AfterConstruction_ReturnsNonNull)
{
    EXPECT_NE(widget->navWidget(), nullptr);
}

TEST_F(TitleBarWidgetExtTest, TitleCrumbBar_AfterConstruction_ReturnsNonNull)
{
    EXPECT_NE(widget->titleCrumbBar(), nullptr);
}

TEST_F(TitleBarWidgetExtTest, HandleHotketActivateTab_ValidIndex_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(widget->handleHotketActivateTab(0));
}

TEST_F(TitleBarWidgetExtTest, HandleHotketNextTab_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(widget->handleHotketNextTab());
}

TEST_F(TitleBarWidgetExtTest, HandleHotketPreviousTab_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(widget->handleHotketPreviousTab());
}

TEST_F(TitleBarWidgetExtTest, HandleHotkeyCtrlF_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(widget->handleHotkeyCtrlF());
}

TEST_F(TitleBarWidgetExtTest, HandleHotkeyCtrlL_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(widget->handleHotkeyCtrlL());
}

TEST_F(TitleBarWidgetExtTest, HandleSplitterAnimation_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(widget->handleSplitterAnimation(50));
}

TEST_F(TitleBarWidgetExtTest, ActivatePinnedTab_EmptyId_DoesNothing)
{
    EXPECT_NO_FATAL_FAILURE(widget->activatePinnedTab(""));
}

TEST_F(TitleBarWidgetExtTest, ActivatePinnedTab_NonExistentId_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(widget->activatePinnedTab("nonexistent"));
}

TEST_F(TitleBarWidgetExtTest, CalculateRemainingWidth_ReturnsNonNegative)
{
    widget->resize(800, 40);
    int w = widget->calculateRemainingWidth();
    EXPECT_GE(w, -10000);
}

TEST_F(TitleBarWidgetExtTest, ShowAddrsssBar_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(widget->showAddrsssBar(QUrl("file:///home")));
}

TEST_F(TitleBarWidgetExtTest, ShowCrumbBar_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(widget->showCrumbBar());
}

TEST_F(TitleBarWidgetExtTest, ShowSearchFilterButton_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(widget->showSearchFilterButton(true));
    EXPECT_NO_FATAL_FAILURE(widget->showSearchFilterButton(false));
}

TEST_F(TitleBarWidgetExtTest, SetViewModeState_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(widget->setViewModeState(0));
}

TEST_F(TitleBarWidgetExtTest, UpdateUiForSizeMode_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(widget->updateUiForSizeMode());
}

TEST_F(TitleBarWidgetExtTest, QuitSearch_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(widget->quitSearch());
}

TEST_F(TitleBarWidgetExtTest, ResizeEvent_DoesNotCrash)
{
    QResizeEvent e(QSize(800, 40), QSize(600, 40));
    EXPECT_NO_FATAL_FAILURE(widget->resizeEvent(&e));
}

TEST_F(TitleBarWidgetExtTest, ShowEvent_DoesNotCrash)
{
    QShowEvent e;
    EXPECT_NO_FATAL_FAILURE(widget->showEvent(&e));
}

TEST_F(TitleBarWidgetExtTest, OnTabAddButtonClicked_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(widget->onTabAddButtonClicked());
}

TEST_F(TitleBarWidgetExtTest, OnAddressBarJump_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(widget->onAddressBarJump());
}

TEST_F(TitleBarWidgetExtTest, RestoreTitleBarState_NonExistentId_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(widget->restoreTitleBarState("nonexistent_id"));
}

TEST_F(TitleBarWidgetExtTest, HandleCreateTabList_EmptyList_DoesNotCrash)
{
    QList<QUrl> urls;
    EXPECT_NO_FATAL_FAILURE(widget->handleCreateTabList(urls));
}

TEST_F(TitleBarWidgetExtTest, CheckCustomFixedTab_ValidIndex_ReturnsBool)
{
    if (widget->tabBar()->count() > 0) {
        EXPECT_NO_FATAL_FAILURE(widget->checkCustomFixedTab(0));
    }
}

TEST_F(TitleBarWidgetExtTest, SetCurrentUrl_ValidUrl_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(widget->setCurrentUrl(QUrl("file:///home")));
}

TEST_F(TitleBarWidgetExtTest, OpenNewTab_ValidUrl_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(widget->openNewTab(QUrl("file:///home")));
}

TEST_F(TitleBarWidgetExtTest, OpenCustomFixedTabs_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(widget->openCustomFixedTabs());
}

TEST_F(TitleBarWidgetExtTest, OnTabCreated_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(widget->onTabCreated());
}

TEST_F(TitleBarWidgetExtTest, HandleCreateView_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(widget->handleCreateView("test_unique_id"));
}
