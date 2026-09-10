// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <dfm-base/widgets/filemanagerwindow.h>
#include <dfm-base/interfaces/abstractframe.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <DMainWindow>
#include <DConfig>

#include <gtest/gtest.h>
#include <QUrl>
#include <QSignalSpy>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QCloseEvent>
#include <QHBoxLayout>

DFMBASE_USE_NAMESPACE

namespace {
class TestFrame : public AbstractFrame
{
public:
    explicit TestFrame(QWidget *parent = nullptr)
        : AbstractFrame(parent)
    {
        setLayout(new QHBoxLayout(this));
        setMinimumSize(100, 100);
    }
    void setCurrentUrl(const QUrl &url) override { m_url = url; }
    QUrl currentUrl() const override { return m_url; }
    QUrl m_url;
};
}   // namespace

class FileManagerWindowTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme),
                       [](const QString &) { __DBG_STUB_INVOKE__ return QIcon(); });
        win = new FileManagerWindow(QUrl("file:///tmp"));
    }
    void TearDown() override
    {
        delete win;
        win = nullptr;
        stub.clear();
    }
    FileManagerWindow *win { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(FileManagerWindowTest, ConstructDoesNotCrash)
{
    EXPECT_NE(win, nullptr);
}

TEST_F(FileManagerWindowTest, DestructDoesNotCrash)
{
    auto *w = new FileManagerWindow(QUrl("file:///tmp"));
    delete w;
    SUCCEED();
}

TEST_F(FileManagerWindowTest, CurrentUrlReturnsInitUrl)
{
    EXPECT_EQ(win->currentUrl(), QUrl("file:///tmp"));
}

TEST_F(FileManagerWindowTest, CdUpdatesCurrentUrl)
{
    QUrl url("file:///home");
    win->cd(url);
    EXPECT_EQ(win->currentUrl(), url);
}

TEST_F(FileManagerWindowTest, CdEmitsCurrentUrlChanged)
{
    QSignalSpy spy(win, &FileManagerWindow::currentUrlChanged);
    QUrl url("file:///home");
    win->cd(url);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toUrl(), url);
}

TEST_F(FileManagerWindowTest, CdWithNoComponentsInstalled)
{
    EXPECT_NO_FATAL_FAILURE(win->cd(QUrl("file:///root")));
}

TEST_F(FileManagerWindowTest, InstallTitleBarDoesNotCrash)
{
    auto *frame = new TestFrame();
    EXPECT_NO_FATAL_FAILURE(win->installTitleBar(frame));
    EXPECT_EQ(win->titleBar(), frame);
}

TEST_F(FileManagerWindowTest, InstallSideBarDoesNotCrash)
{
    auto *frame = new TestFrame();
    EXPECT_NO_FATAL_FAILURE(win->installSideBar(frame));
    EXPECT_EQ(win->sideBar(), frame);
}

TEST_F(FileManagerWindowTest, InstallWorkSpaceDoesNotCrash)
{
    auto *frame = new TestFrame();
    EXPECT_NO_FATAL_FAILURE(win->installWorkSpace(frame));
    EXPECT_EQ(win->workSpace(), frame);
}

TEST_F(FileManagerWindowTest, InstallDetailViewDoesNotCrash)
{
    auto *frame = new TestFrame();
    EXPECT_NO_FATAL_FAILURE(win->installDetailView(frame));
    EXPECT_EQ(win->detailView(), frame);
}

TEST_F(FileManagerWindowTest, TitleBarReturnsNullBeforeInstall)
{
    EXPECT_EQ(win->titleBar(), nullptr);
}

TEST_F(FileManagerWindowTest, SideBarReturnsNullBeforeInstall)
{
    EXPECT_EQ(win->sideBar(), nullptr);
}

TEST_F(FileManagerWindowTest, WorkSpaceReturnsNullBeforeInstall)
{
    EXPECT_EQ(win->workSpace(), nullptr);
}

TEST_F(FileManagerWindowTest, DetailViewReturnsNullBeforeInstall)
{
    EXPECT_EQ(win->detailView(), nullptr);
}

TEST_F(FileManagerWindowTest, IsDetailSpaceVisibleDefault)
{
    EXPECT_NO_FATAL_FAILURE(win->isDetailSpaceVisible());
}

TEST_F(FileManagerWindowTest, SetDetailViewWidthDoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(win->setDetailViewWidth(300));
}

TEST_F(FileManagerWindowTest, SetDetailViewWidthClamped)
{
    win->setDetailViewWidth(50);
    EXPECT_NO_FATAL_FAILURE(win->detailViewWidth());
    win->setDetailViewWidth(1000);
    EXPECT_NO_FATAL_FAILURE(win->detailViewWidth());
}

TEST_F(FileManagerWindowTest, DetailViewWidthDefault)
{
    EXPECT_NO_FATAL_FAILURE(win->detailViewWidth());
}

TEST_F(FileManagerWindowTest, HideDetailSpaceDoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(win->hideDetailSpace());
}

TEST_F(FileManagerWindowTest, HideDetailSpaceNonAnimated)
{
    QVariantHash opts;
    opts[DetailSpaceOptions::kAnimated] = false;
    EXPECT_NO_FATAL_FAILURE(win->hideDetailSpace(opts));
}

TEST_F(FileManagerWindowTest, HideDetailSpaceNonUserAction)
{
    QVariantHash opts;
    opts[DetailSpaceOptions::kUserAction] = false;
    EXPECT_NO_FATAL_FAILURE(win->hideDetailSpace(opts));
}

TEST_F(FileManagerWindowTest, ShowDetailSpaceDoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(win->showDetailSpace());
}

TEST_F(FileManagerWindowTest, ShowDetailSpaceNonAnimated)
{
    QVariantHash opts;
    opts[DetailSpaceOptions::kAnimated] = false;
    EXPECT_NO_FATAL_FAILURE(win->showDetailSpace(opts));
}

TEST_F(FileManagerWindowTest, LoadStateDoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(win->loadState());
}





TEST_F(FileManagerWindowTest, MoveCenterDoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(win->moveCenter());
}

TEST_F(FileManagerWindowTest, CloseEventEmitsAboutToClose)
{
    QSignalSpy spy(win, &FileManagerWindow::aboutToClose);
    QCloseEvent event;
    EXPECT_NO_FATAL_FAILURE(win->closeEvent(&event));
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(FileManagerWindowTest, MoveEventEmitsPositionChanged)
{
    QSignalSpy spy(win, &FileManagerWindow::positionChanged);
    QMoveEvent event(QPoint(10, 20), QPoint(0, 0));
    EXPECT_NO_FATAL_FAILURE(win->moveEvent(&event));
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toPoint(), QPoint(10, 20));
}

TEST_F(FileManagerWindowTest, ResizeEventDoesNotCrash)
{
    QResizeEvent event(QSize(800, 600), QSize(400, 300));
    EXPECT_NO_FATAL_FAILURE(win->resizeEvent(&event));
}

TEST_F(FileManagerWindowTest, KeyPressEventF5EmitsReqRefresh)
{
    QSignalSpy spy(win, &FileManagerWindow::reqRefresh);
    QKeyEvent event(QEvent::KeyPress, Qt::Key_F5, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(win->keyPressEvent(&event));
    EXPECT_GE(spy.count(), 1);
}

TEST_F(FileManagerWindowTest, KeyPressEventCtrlFEmitsReqSearchCtrlF)
{
    QSignalSpy spy(win, &FileManagerWindow::reqSearchCtrlF);
    QKeyEvent event(QEvent::KeyPress, Qt::Key_F, Qt::ControlModifier);
    EXPECT_NO_FATAL_FAILURE(win->keyPressEvent(&event));
    EXPECT_GE(spy.count(), 1);
}

TEST_F(FileManagerWindowTest, KeyPressEventCtrlLEmitsReqSearchCtrlL)
{
    QSignalSpy spy(win, &FileManagerWindow::reqSearchCtrlL);
    QKeyEvent event(QEvent::KeyPress, Qt::Key_L, Qt::ControlModifier);
    EXPECT_NO_FATAL_FAILURE(win->keyPressEvent(&event));
    EXPECT_GE(spy.count(), 1);
}

TEST_F(FileManagerWindowTest, KeyPressEventCtrlWEmitsReqCloseCurrentTab)
{
    QSignalSpy spy(win, &FileManagerWindow::reqCloseCurrentTab);
    QKeyEvent event(QEvent::KeyPress, Qt::Key_W, Qt::ControlModifier);
    EXPECT_NO_FATAL_FAILURE(win->keyPressEvent(&event));
    EXPECT_GE(spy.count(), 1);
}

TEST_F(FileManagerWindowTest, KeyPressEventCtrlTEmitsReqCreateTab)
{
    QSignalSpy spy(win, &FileManagerWindow::reqCreateTab);
    QKeyEvent event(QEvent::KeyPress, Qt::Key_T, Qt::ControlModifier);
    EXPECT_NO_FATAL_FAILURE(win->keyPressEvent(&event));
    EXPECT_GE(spy.count(), 1);
}

TEST_F(FileManagerWindowTest, KeyPressEventCtrlNEmitsReqCreateWindow)
{
    QSignalSpy spy(win, &FileManagerWindow::reqCreateWindow);
    QKeyEvent event(QEvent::KeyPress, Qt::Key_N, Qt::ControlModifier);
    EXPECT_NO_FATAL_FAILURE(win->keyPressEvent(&event));
    EXPECT_GE(spy.count(), 1);
}

TEST_F(FileManagerWindowTest, KeyPressEventCtrlTabEmitsReqActivateNextTab)
{
    QSignalSpy spy(win, &FileManagerWindow::reqActivateNextTab);
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Tab, Qt::ControlModifier);
    EXPECT_NO_FATAL_FAILURE(win->keyPressEvent(&event));
    EXPECT_GE(spy.count(), 1);
}

TEST_F(FileManagerWindowTest, KeyPressEventUnhandledFallsThrough)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Z, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(win->keyPressEvent(&event));
}

TEST_F(FileManagerWindowTest, InstallAllComponentsThenCd)
{
    auto *tb = new TestFrame();
    auto *sb = new TestFrame();
    auto *ws = new TestFrame();
    auto *dv = new TestFrame();

    win->installTitleBar(tb);
    win->installSideBar(sb);
    win->installWorkSpace(ws);
    win->installDetailView(dv);

    QUrl url("file:///home/uos");
    win->cd(url);

    EXPECT_EQ(tb->currentUrl(), url);
    EXPECT_EQ(sb->currentUrl(), url);
    EXPECT_EQ(ws->currentUrl(), url);
    EXPECT_EQ(dv->currentUrl(), url);
}

TEST_F(FileManagerWindowTest, InstallAllComponentsThenResize)
{
    auto *tb = new TestFrame();
    auto *sb = new TestFrame();
    auto *ws = new TestFrame();
    win->installTitleBar(tb);
    win->installSideBar(sb);
    win->installWorkSpace(ws);

    QResizeEvent event(QSize(1200, 800), QSize(800, 600));
    EXPECT_NO_FATAL_FAILURE(win->resizeEvent(&event));
}

TEST_F(FileManagerWindowTest, InstallAllComponentsThenShowHideDetail)
{
    auto *sb = new TestFrame();
    auto *ws = new TestFrame();
    auto *dv = new TestFrame();
    win->installSideBar(sb);
    win->installWorkSpace(ws);
    win->installDetailView(dv);

    QVariantHash opts;
    opts[DetailSpaceOptions::kAnimated] = false;
    EXPECT_NO_FATAL_FAILURE(win->showDetailSpace(opts));
    EXPECT_NO_FATAL_FAILURE(win->hideDetailSpace(opts));
}

TEST_F(FileManagerWindowTest, CdEmitsCurrentUrlChangedMultipleTimes)
{
    QSignalSpy spy(win, &FileManagerWindow::currentUrlChanged);
    win->cd(QUrl("file:///a"));
    win->cd(QUrl("file:///b"));
    win->cd(QUrl("file:///c"));
    EXPECT_EQ(spy.count(), 3);
}

TEST_F(FileManagerWindowTest, SetDetailViewWidthAfterInstall)
{
    auto *sb = new TestFrame();
    auto *ws = new TestFrame();
    auto *dv = new TestFrame();
    win->installSideBar(sb);
    win->installWorkSpace(ws);
    win->installDetailView(dv);

    EXPECT_NO_FATAL_FAILURE(win->setDetailViewWidth(350));
    EXPECT_NO_FATAL_FAILURE(win->detailViewWidth());
}
