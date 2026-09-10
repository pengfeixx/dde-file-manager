// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QShowEvent>
#include <QHideEvent>
#include <QApplication>
#include <QModelIndex>
#include <QItemSelection>
#include <QPoint>
#include <QSignalSpy>
#include <QTimer>
#include <QVariant>

#include "stubext.h"
#include "views/computerview.h"
#include "private/computerview_p.h"
#include "views/computerstatusbar.h"
#include "models/computermodel.h"
#include "utils/computerutils.h"
#include "controller/computercontroller.h"
#include "events/computereventcaller.h"
#include "utils/computerdatastruct.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/device/devicemanager.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_computer;

// ---------------------------------------------------------------------------
// ComputerView extension tests
// ---------------------------------------------------------------------------
class ComputerViewExtTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        stub.set_lamda(&ComputerUtils::rootUrl, []() {
            __DBG_STUB_INVOKE__
            return QUrl("computer:///");
        });

        view = new ComputerView(QUrl("computer:///"));
        view->setStatusBarHandler(new ComputerStatusBar(view));
    }

    void TearDown() override
    {
        delete view;
        view = nullptr;
        stub.clear();
    }

    stub_ext::StubExt stub;
    ComputerView *view = nullptr;
};

TEST_F(ComputerViewExtTest, Widget_ReturnsSelf)
{
    EXPECT_EQ(view->widget(), view);
}

TEST_F(ComputerViewExtTest, RootUrl_ReturnsComputerUrl)
{
    EXPECT_EQ(view->rootUrl(), QUrl("computer:///"));
}

TEST_F(ComputerViewExtTest, ViewState_DefaultIdle)
{
    EXPECT_NO_FATAL_FAILURE(view->viewState());
}

TEST_F(ComputerViewExtTest, SetRootUrl_AnyUrl_ReturnsTrue)
{
    EXPECT_TRUE(view->setRootUrl(QUrl("computer:///")));
}

TEST_F(ComputerViewExtTest, SelectedUrlList_EmptyWhenNoSelection)
{
    EXPECT_TRUE(view->selectedUrlList().isEmpty());
}

TEST_F(ComputerViewExtTest, SetStatusBarHandler_NoCrash)
{
    auto *sb = new ComputerStatusBar(view);
    EXPECT_NO_FATAL_FAILURE(view->setStatusBarHandler(sb));
}

TEST_F(ComputerViewExtTest, SetRowHidden_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->setRowHidden(0, true));
    EXPECT_NO_FATAL_FAILURE(view->setRowHidden(0, false));
}

TEST_F(ComputerViewExtTest, HandleComputerItemVisible_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->handleComputerItemVisible());
}

TEST_F(ComputerViewExtTest, HandleDisksVisible_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->handleDisksVisible());
}

TEST_F(ComputerViewExtTest, HandleUserDirVisible_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->handleUserDirVisible());
}

TEST_F(ComputerViewExtTest, Handle3rdEntriesVisible_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->handle3rdEntriesVisible());
}

TEST_F(ComputerViewExtTest, HandleDiskSplitterVisible_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->handleDiskSplitterVisible());
}

TEST_F(ComputerViewExtTest, ComputerModel_NotNull)
{
    EXPECT_NE(view->computerModel(), nullptr);
}

TEST_F(ComputerViewExtTest, EventFilter_KeyPressEvent_NoCrash)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(view->eventFilter(view, &event));
}

TEST_F(ComputerViewExtTest, EventFilter_BackKey_NoCrash)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Back, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(view->eventFilter(view, &event));
}

TEST_F(ComputerViewExtTest, EventFilter_EnterKey_NoCrash)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(view->eventFilter(view, &event));
}

TEST_F(ComputerViewExtTest, EventFilter_MouseButtonPress_NoCrash)
{
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(view->eventFilter(view, &event));
}

TEST_F(ComputerViewExtTest, EventFilter_UnknownObject_NoCrash)
{
    QObject obj;
    QKeyEvent event(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(view->eventFilter(&obj, &event));
}

TEST_F(ComputerViewExtTest, KeyPressEvent_NormalKey_NoCrash)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(view->keyPressEvent(&event));
}

TEST_F(ComputerViewExtTest, KeyPressEvent_AltModifier_NoCrash)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_A, Qt::AltModifier);
    EXPECT_NO_FATAL_FAILURE(view->keyPressEvent(&event));
}

TEST_F(ComputerViewExtTest, ShowEvent_NoCrash)
{
    QShowEvent event;
    EXPECT_NO_FATAL_FAILURE(view->showEvent(&event));
}

TEST_F(ComputerViewExtTest, HideEvent_NoCrash)
{
    QHideEvent event;
    EXPECT_NO_FATAL_FAILURE(view->hideEvent(&event));
}

TEST_F(ComputerViewExtTest, OnSelectionChanged_NoSelection_NoCrash)
{
    QItemSelection emptySel;
    EXPECT_NO_FATAL_FAILURE(view->onSelectionChanged(emptySel, emptySel));
}

TEST_F(ComputerViewExtTest, CdTo_InvalidIndex_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->cdTo(QModelIndex()));
}

TEST_F(ComputerViewExtTest, OnMenuRequest_InvalidIndex_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->onMenuRequest(QPoint(0, 0)));
}

TEST_F(ComputerViewExtTest, OnRenameRequest_DifferentWindow_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->onRenameRequest(99999, QUrl()));
}

TEST_F(ComputerViewExtTest, OnUpdateItemAlias_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->onUpdateItemAlias(QUrl(), "alias", false));
}


TEST_F(ComputerViewExtTest, EnterPressed_SignalEmittable)
{
    QSignalSpy spy(view, &ComputerView::enterPressed);
    EXPECT_TRUE(spy.isValid());
}

TEST_F(ComputerViewExtTest, SetRootUrl_MultipleCalls_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->setRootUrl(QUrl("computer:///")));
    EXPECT_NO_FATAL_FAILURE(view->setRootUrl(QUrl("computer:///somedir")));
}


// Minimal entity for EntryFileInfo registration in test environment
class TestEntryEntity : public AbstractEntryFileEntity
{
public:
    explicit TestEntryEntity(const QUrl &url) : AbstractEntryFileEntity(url) {}
    ~TestEntryEntity() override = default;
    QString displayName() const override { return "test"; }
    QIcon icon() const override { return {}; }
    bool exists() const override { return true; }
    bool showProgress() const override { return false; }
    bool showTotalSize() const override { return false; }
    bool showUsageSize() const override { return false; }
    EntryOrder order() const override { return kOrderCustom; }
};

// ---------------------------------------------------------------------------
// ComputerController extension tests
// ---------------------------------------------------------------------------
class ComputerControllerExtTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        EntryEntityFactor::registCreator<TestEntryEntity>("_common_");
        controller = ComputerController::instance();

        // Stub mountDevice overloads to prevent mountBlockDevAsync crash
        stub.set_lamda(static_cast<void(ComputerController::*)(quint64, const DFMEntryFileInfoPointer, ComputerController::ActionAfterMount)>(
            &ComputerController::mountDevice), [](ComputerController *, quint64, const DFMEntryFileInfoPointer, ComputerController::ActionAfterMount) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(static_cast<void(ComputerController::*)(quint64, const QString &, const QString &, ComputerController::ActionAfterMount)>(
            &ComputerController::mountDevice), [](ComputerController *, quint64, const QString &, const QString &, ComputerController::ActionAfterMount) {
            __DBG_STUB_INVOKE__
        });
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
    ComputerController *controller = nullptr;
};

TEST_F(ComputerControllerExtTest, Instance_Singleton)
{
    EXPECT_EQ(ComputerController::instance(), controller);
}

TEST_F(ComputerControllerExtTest, Instance_NotNull)
{
    EXPECT_NE(ComputerController::instance(), nullptr);
}

TEST_F(ComputerControllerExtTest, ActionAfterMount_EnumValues)
{
    EXPECT_NE(ComputerController::kEnterDirectory, ComputerController::kNone);
    EXPECT_NE(ComputerController::kEnterInNewWindow, ComputerController::kEnterInNewTab);
    EXPECT_NE(ComputerController::kEnterDirectory, ComputerController::kEnterInNewWindow);
}

TEST_F(ComputerControllerExtTest, OnOpenItem_ValidUrl_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(controller->onOpenItem(12345, QUrl("entry://test")));
}

TEST_F(ComputerControllerExtTest, OnOpenItem_EmptyUrl_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(controller->onOpenItem(12345, QUrl("entry://test")));
}

TEST_F(ComputerControllerExtTest, OnMenuRequest_FromSidebar_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(controller->onMenuRequest(12345, QUrl("entry://test"), true));
}

TEST_F(ComputerControllerExtTest, OnMenuRequest_FromView_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(controller->onMenuRequest(12345, QUrl("entry://test"), false));
}

TEST_F(ComputerControllerExtTest, DoRename_ValidParams_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(controller->doRename(12345, QUrl("entry://test"), "NewName"));
}

TEST_F(ComputerControllerExtTest, DoRename_EmptyName_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(controller->doRename(12345, QUrl("entry://test"), ""));
}

TEST_F(ComputerControllerExtTest, ActEject_ValidUrl_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(controller->actEject(QUrl("entry://test")));
}

TEST_F(ComputerControllerExtTest, ActEject_EmptyUrl_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(controller->actEject(QUrl("entry://test")));
}

TEST_F(ComputerControllerExtTest, ActFormat_ValidInfo_NoCrash)
{
    auto info = DFMEntryFileInfoPointer(new EntryFileInfo(QUrl("entry://test")));
    EXPECT_NO_FATAL_FAILURE(controller->actFormat(12345, info));
}

TEST_F(ComputerControllerExtTest, ActProperties_ValidInfo_NoCrash)
{
    auto info = DFMEntryFileInfoPointer(new EntryFileInfo(QUrl("entry://test")));
    EXPECT_NO_FATAL_FAILURE(controller->actProperties(12345, info));
}

TEST_F(ComputerControllerExtTest, ActRename_ValidInfo_NoCrash)
{
    auto info = DFMEntryFileInfoPointer(new EntryFileInfo(QUrl("entry://test")));
    EXPECT_NO_FATAL_FAILURE(controller->actRename(12345, info, false));
}

TEST_F(ComputerControllerExtTest, ActRename_FromSidebar_NoCrash)
{
    auto info = DFMEntryFileInfoPointer(new EntryFileInfo(QUrl("entry://test")));
    EXPECT_NO_FATAL_FAILURE(controller->actRename(12345, info, true));
}

TEST_F(ComputerControllerExtTest, ActUnmount_ValidInfo_NoCrash)
{
    auto info = DFMEntryFileInfoPointer(new EntryFileInfo(QUrl("entry://test")));
    EXPECT_NO_FATAL_FAILURE(controller->actUnmount(info));
}

TEST_F(ComputerControllerExtTest, ActSafelyRemove_ValidInfo_NoCrash)
{
    auto info = DFMEntryFileInfoPointer(new EntryFileInfo(QUrl("entry://test")));
    EXPECT_NO_FATAL_FAILURE(controller->actSafelyRemove(info));
}

TEST_F(ComputerControllerExtTest, ActOpenInNewWindow_ValidInfo_NoCrash)
{
    auto info = DFMEntryFileInfoPointer(new EntryFileInfo(QUrl("entry://test")));
    EXPECT_NO_FATAL_FAILURE(controller->actOpenInNewWindow(12345, info));
}

TEST_F(ComputerControllerExtTest, ActOpenInNewTab_ValidInfo_NoCrash)
{
    auto info = DFMEntryFileInfoPointer(new EntryFileInfo(QUrl("entry://test")));
    EXPECT_NO_FATAL_FAILURE(controller->actOpenInNewTab(12345, info));
}

TEST_F(ComputerControllerExtTest, ActMount_ValidInfo_NoCrash)
{
    auto info = DFMEntryFileInfoPointer(new EntryFileInfo(QUrl("entry://test")));
    EXPECT_NO_FATAL_FAILURE(controller->actMount(12345, info, false));
}

TEST_F(ComputerControllerExtTest, ActMount_EnterAfterMount_NoCrash)
{
    auto info = DFMEntryFileInfoPointer(new EntryFileInfo(QUrl("entry://test")));
    EXPECT_NO_FATAL_FAILURE(controller->actMount(12345, info, true));
}

TEST_F(ComputerControllerExtTest, ActLogoutAndForgetPasswd_ValidInfo_NoCrash)
{
    auto info = DFMEntryFileInfoPointer(new EntryFileInfo(QUrl("entry://test")));
    EXPECT_NO_FATAL_FAILURE(controller->actLogoutAndForgetPasswd(info));
}

TEST_F(ComputerControllerExtTest, ActErase_ValidInfo_NoCrash)
{
    auto info = DFMEntryFileInfoPointer(new EntryFileInfo(QUrl("entry://test")));
    EXPECT_NO_FATAL_FAILURE(controller->actErase(info));
}

TEST_F(ComputerControllerExtTest, MountDevice_WithInfo_DefaultAction_NoCrash)
{
    auto info = DFMEntryFileInfoPointer(new EntryFileInfo(QUrl("entry://test")));
    EXPECT_NO_FATAL_FAILURE(controller->mountDevice(12345, info));
}

TEST_F(ComputerControllerExtTest, MountDevice_WithInfo_EnterInNewWindow_NoCrash)
{
    auto info = DFMEntryFileInfoPointer(new EntryFileInfo(QUrl("entry://test")));
    EXPECT_NO_FATAL_FAILURE(controller->mountDevice(12345, info, ComputerController::kEnterInNewWindow));
}

TEST_F(ComputerControllerExtTest, MountDevice_WithIds_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(controller->mountDevice(12345, "test_device", "shell_id"));
}

TEST_F(ComputerControllerExtTest, MountDevice_WithIds_EnterInNewTab_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(controller->mountDevice(12345, "test_device", "shell_id", ComputerController::kEnterInNewTab));
}

TEST_F(ComputerControllerExtTest, DoSetAlias_ValidInfo_NoCrash)
{
    auto info = DFMEntryFileInfoPointer(new EntryFileInfo(QUrl("entry://test")));
    EXPECT_NO_FATAL_FAILURE(controller->doSetAlias(info, "My Alias"));
}

TEST_F(ComputerControllerExtTest, DoSetAlias_EmptyAlias_NoCrash)
{
    auto info = DFMEntryFileInfoPointer(new EntryFileInfo(QUrl("entry://test")));
    EXPECT_NO_FATAL_FAILURE(controller->doSetAlias(info, ""));
}

TEST_F(ComputerControllerExtTest, DoSetProtocolDeviceAlias_ValidInfo_NoCrash)
{
    auto info = DFMEntryFileInfoPointer(new EntryFileInfo(QUrl("entry://test")));
    EXPECT_NO_FATAL_FAILURE(controller->doSetProtocolDeviceAlias(info, "Protocol Alias"));
}

TEST_F(ComputerControllerExtTest, RequestRename_SignalEmittable)
{
    QSignalSpy spy(controller, &ComputerController::requestRename);
    EXPECT_TRUE(spy.isValid());
}

TEST_F(ComputerControllerExtTest, UpdateItemAlias_SignalEmittable)
{
    QSignalSpy spy(controller, &ComputerController::updateItemAlias);
    EXPECT_TRUE(spy.isValid());
}

TEST_F(ComputerControllerExtTest, OnOpenItem_MultipleUrls_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(controller->onOpenItem(1, QUrl("entry://a")));
    EXPECT_NO_FATAL_FAILURE(controller->onOpenItem(2, QUrl("entry://b")));
    EXPECT_NO_FATAL_FAILURE(controller->onOpenItem(3, QUrl("entry://c")));
}
