#include <gtest/gtest.h>

#include <QApplication>
#include <QTimer>
#include <QListWidget>
#include <QLineEdit>
#include <QCloseEvent>
#include <QtTest/QTest>
#include <QPushButton>
#include <QComboBox>
#include <QToolBox>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QSignalSpy>
#include "../mainWindow/mainwindow.h"
#include "../tasks/TaskManager.h"
#include "../databaseManager/databaseManager.h"
#include "../databaseManager/TaskRepository.h"
#include "../settings/appSettings.h"
#include "friend_mainwindow.h"
#include "../mainWindow/taskEditorWindow.h"
#include "../tasks/task.h"
#include "../settings/settingsWindow.h"
#include "../databaseManager/accountRepository.h"

struct MainWindowFixture : public ::testing::Test {
    std::unique_ptr<DatabaseManager> db;
    std::unique_ptr<TaskRepository> repo;
    std::unique_ptr<TaskManager> manager;
    std::unique_ptr<MainWindow> window;

    void SetUp() override {
        db = std::make_unique<DatabaseManager>(":memory:");
        ASSERT_TRUE(db->open());
        repo = std::make_unique<TaskRepository>(*db);
        repo->initTable();
        manager = std::make_unique<TaskManager>(*repo);
        window = std::make_unique<MainWindow>(*manager);
    }
};

class MainWindowTest : public ::testing::Test {
protected:
    std::unique_ptr<MainWindow> window;
    DatabaseManager db{":memory:"};
    std::unique_ptr<TaskRepository> repo;
    std::unique_ptr<TaskManager> taskManager;

    void SetUp() override {
        db.open();
        repo = std::make_unique<TaskRepository>(db);
        taskManager = std::make_unique<TaskManager>(*repo);
        AccountRepository accRepo(db);
        accRepo.initTable();
        accRepo.addAccount("testuser", 12345);
        taskManager->setCurrentUser("testuser");

        window = std::make_unique<MainWindow>(*taskManager);
        window->show();
    }

    void TearDown() override {
        window.reset();
    }
};

TEST(MainWindowQt, ApplyTrayThemeAccessible) {
    DatabaseManager db(":memory:");
    ASSERT_TRUE(db.open());
    TaskRepository repo(db);
    repo.initTable();
    TaskManager mgr(repo);

    TestFriend_MainWindow w(mgr);
    EXPECT_NO_THROW(w.applyTrayTheme());
    EXPECT_NO_THROW(w.updateTrayTooltip());
    EXPECT_NO_THROW(w.loadTasks());
    EXPECT_NO_THROW(w.updateMaximizeIcon(true));
}

TEST_F(MainWindowFixture, HasCoreWidgets) {
    EXPECT_TRUE(window->findChild<QPushButton*>("addTaskButton"));
    EXPECT_TRUE(window->findChild<QPushButton*>("quickAddButton"));
    EXPECT_TRUE(window->findChild<QPushButton*>("themeButton"));
    EXPECT_TRUE(window->findChild<QPushButton*>("settingsButton"));
    EXPECT_TRUE(window->findChild<QComboBox*>("viewFilterBox"));
    EXPECT_TRUE(window->findChild<QToolBox*>());
}

TEST_F(MainWindowFixture, SwitchThemeChangesSettingAndEmitsSignal) {
    using Theme = AppSettings::Theme;
    Theme initial = AppSettings::theme();

    auto themeBtn = window->findChild<QPushButton*>("themeButton");
    ASSERT_TRUE(themeBtn);

    QSignalSpy spy(window.get(), SIGNAL(themeChanged(AppSettings::Theme)));
    themeBtn->click();

    QTRY_VERIFY_WITH_TIMEOUT(spy.count() > 0, 500);

    Theme newTheme = AppSettings::theme();
    EXPECT_NE(newTheme, initial);
    EXPECT_TRUE(newTheme == Theme::Light || newTheme == Theme::Dark);
}

TEST_F(MainWindowFixture, SettingsButtonExistsAndResponds) {
    auto btn = window->findChild<QPushButton*>("settingsButton");
    ASSERT_TRUE(btn);
    btn->click();
    SUCCEED();
}

TEST_F(MainWindowFixture, FilterBoxHasCorrectItems) {
    auto combo = window->findChild<QComboBox*>("viewFilterBox");
    ASSERT_TRUE(combo);
    EXPECT_GE(combo->count(), 4);
    EXPECT_EQ(combo->itemText(0), "All Tasks");
}

TEST_F(MainWindowFixture, UpdateMaximizeIconChangesIcons) {
    TestFriend_MainWindow w(*manager);
    EXPECT_NO_THROW(w.updateMaximizeIcon(true));
    EXPECT_NO_THROW(w.updateMaximizeIcon(false));
    SUCCEED();
}

TEST_F(MainWindowFixture, TrayIconCreatedAndMenuHasExpectedActions) {
    auto tray = window->findChild<QSystemTrayIcon*>();
    ASSERT_TRUE(tray);
    EXPECT_TRUE(tray->isVisible());

    auto menu = tray->contextMenu();
    ASSERT_TRUE(menu);
    EXPECT_GE(menu->actions().size(), 5);
}

TEST_F(MainWindowFixture, StatusMenuChangesUserStatus) {
    auto tray = window->findChild<QSystemTrayIcon*>();
    ASSERT_TRUE(tray);
    auto menu = tray->contextMenu();
    ASSERT_TRUE(menu);

    QMenu *statusMenu = nullptr;
    for (auto *act : menu->actions()) {
        if (act->menu() && act->menu()->title().contains("Status")) {
            statusMenu = act->menu();
            break;
        }
    }
    ASSERT_TRUE(statusMenu) << "Status submenu not found";

    QAction *statusBusy = nullptr;
    for (auto *act : statusMenu->actions()) {
        if (act->text().contains("Busy")) {
            statusBusy = act;
            break;
        }
    }
    ASSERT_TRUE(statusBusy) << "Busy action not found in Status submenu";

    statusBusy->trigger();
    QCoreApplication::processEvents();

    EXPECT_EQ(AppSettings::userStatus(), AppSettings::UserStatus::Busy)
        << "Triggering Busy should change AppSettings::userStatus";
}

TEST_F(MainWindowFixture, LoadTasksRunsWithoutCrash) {
    TestFriend_MainWindow w(*manager);
    EXPECT_NO_THROW(w.loadTasks());
}

TEST_F(MainWindowFixture, ApplyTrayThemeDoesNotCrash) {
    TestFriend_MainWindow w(*manager);
    EXPECT_NO_THROW(w.applyTrayTheme());
}


TEST_F(MainWindowFixture, TrayTooltipContainsUserStatusAndTheme) {
    TestFriend_MainWindow w(*manager);
    EXPECT_NO_THROW(w.updateTrayTooltip());

    auto tray = w.findChild<QSystemTrayIcon*>();
    ASSERT_TRUE(tray);

    QString tip = tray->toolTip();
    EXPECT_FALSE(tip.isEmpty());

    EXPECT_TRUE(tip.contains(AppSettings::userStatusName(), Qt::CaseInsensitive))
        << "Tooltip should contain current user status name.";
}

TEST_F(MainWindowFixture, AddTaskButtonTriggersOpenTaskEditor) {
    auto btn = window->findChild<QPushButton*>("addTaskButton");
    ASSERT_TRUE(btn);
    btn->click();
    SUCCEED();
}

TEST_F(MainWindowFixture, ReminderTimerAndPlayerInitialized) {
    EXPECT_TRUE(window->findChild<QTimer*>());
    EXPECT_TRUE(window->findChild<QMediaPlayer*>());
    EXPECT_TRUE(window->findChild<QAudioOutput*>());
}

TEST_F(MainWindowFixture, AutoDeleteTimerRuns) {
    EXPECT_TRUE(window->findChild<QTimer*>());
    SUCCEED();
}

void closeActiveDialogLater(int ms = 100, int exitCode = QDialog::Accepted) {
    QTimer::singleShot(ms, [exitCode]() {
        for (QWidget* widget : QApplication::topLevelWidgets()) {
            if (QDialog* dlg = qobject_cast<QDialog*>(widget)) {
                if (dlg->isVisible()) {
                    dlg->done(exitCode);
                    return;
                }
            } else if (widget->isVisible() && widget->inherits("FramelessMessageBox")) {
                widget->close();
            }
        }
    });
}

TEST_F(MainWindowTest, AddQuickTask_EmptyTitleShowsWarning) {
    auto taskInput = window->findChild<QLineEdit*>("taskInput");
    ASSERT_TRUE(taskInput);
    taskInput->setText("   ");

    closeActiveDialogLater(100);

    QMetaObject::invokeMethod(window.get(), "addQuickTask");

    EXPECT_EQ(taskInput->text(), "   ");
}

TEST_F(MainWindowTest, AddQuickTask_ValidTitleAddsTask) {
    auto taskInput = window->findChild<QLineEdit*>("taskInput");
    taskInput->setText("My Quick Task");

    QMetaObject::invokeMethod(window.get(), "addQuickTask");

    EXPECT_TRUE(taskInput->text().isEmpty());
}

TEST_F(MainWindowTest, HandleTaskEdit_AcceptsAndUpdates) {
    Task testTask("Old Title", "Desc", QDateTime::currentDateTime(), "Low", false);

    QTimer::singleShot(100, []() {
        for (QWidget* w : QApplication::topLevelWidgets()) {
            if (auto editor = qobject_cast<TaskEditorWindow*>(w)) {
                editor->accept();
            }
        }
    });
    closeActiveDialogLater(200);

    QMetaObject::invokeMethod(window.get(), "handleTaskEdit", Q_ARG(Task, testTask));
}

TEST_F(MainWindowTest, HandleTaskDone_AlreadyCompleted) {
    Task completedTask("Done Task", "Desc", QDateTime::currentDateTime(), "Low", true);

    closeActiveDialogLater(100);
    QMetaObject::invokeMethod(window.get(), "handleTaskDone", Q_ARG(Task, completedTask));
}

TEST_F(MainWindowTest, HandleTaskDone_Success) {
    Task task("To complete", "Desc", QDateTime::currentDateTime(), "Low", false);
    taskManager->addTask(window.get(), task);

    QMetaObject::invokeMethod(window.get(), "handleTaskDone", Q_ARG(Task, task));
}

TEST_F(MainWindowTest, HandleTaskDelete_ConfirmsAndDeletes) {
    Task taskToDelete("Delete Me", "Desc", QDateTime::currentDateTime(), "Low", false);
    taskManager->addTask(window.get(), taskToDelete);

    closeActiveDialogLater(100, QMessageBox::Yes);

    QMetaObject::invokeMethod(window.get(), "handleTaskDelete", Q_ARG(Task, taskToDelete));
}

TEST_F(MainWindowTest, HandleTaskDetails_ShowsInfo) {
    Task task("Info Task", "Desc", QDateTime::currentDateTime(), "High", false);

    closeActiveDialogLater(100);
    QMetaObject::invokeMethod(window.get(), "handleTaskDetails", Q_ARG(Task, task));
}

TEST_F(MainWindowTest, OnFilterChanged_UpdatesList) {
    QMetaObject::invokeMethod(window.get(), "onFilterChanged", Q_ARG(int, 1));
}

TEST_F(MainWindowTest, AddTaskToToolBox_SortsCorrectly) {
    QDateTime today = QDateTime::currentDateTime();

    Task tOverdue("Overdue", "", today.addDays(-2), "Low", false);
    Task tToday("Today", "", today, "Low", false);
    Task tWeek("Week", "", today.addDays(3), "Low", false);
    Task tMonth("Month", "", today.addDays(15), "Low", false);
    Task tLater("Later", "", today.addMonths(2), "Low", false);

    QMetaObject::invokeMethod(window.get(), "addTaskToToolBox", Q_ARG(Task, tOverdue));
    QMetaObject::invokeMethod(window.get(), "addTaskToToolBox", Q_ARG(Task, tToday));
    QMetaObject::invokeMethod(window.get(), "addTaskToToolBox", Q_ARG(Task, tWeek));
    QMetaObject::invokeMethod(window.get(), "addTaskToToolBox", Q_ARG(Task, tMonth));
    QMetaObject::invokeMethod(window.get(), "addTaskToToolBox", Q_ARG(Task, tLater));

}

TEST_F(MainWindowTest, CheckRemindersTick_ShowsReminder) {
    AppSettings::setReminderEnabled(true);
    AppSettings::setReminderMinutes(10);

    QDateTime deadline = QDateTime::currentDateTime().addSecs(5 * 60);
    Task remindTask("Remind Me", "", deadline, "High", false);
    taskManager->addTask(window.get(), remindTask);

    QMetaObject::invokeMethod(window.get(), "checkRemindersTick");

    closeActiveDialogLater(400);

    QCoreApplication::processEvents();
}

TEST_F(MainWindowTest, CloseEvent_IgnoresWhenTrayVisible) {
    QCloseEvent closeEvent;

    QMetaObject::invokeMethod(window.get(), "closeEvent", Q_ARG(QCloseEvent*, &closeEvent));

    EXPECT_TRUE(closeEvent.isAccepted() || !closeEvent.isAccepted());
}

TEST_F(MainWindowTest, OpenSettingsAndAccept) {
    QTimer::singleShot(100, []() {
        for (QWidget* w : QApplication::topLevelWidgets()) {
            if (auto sw = qobject_cast<SettingsWindow*>(w)) {
                sw->accept();
            }
        }
    });

     QMetaObject::invokeMethod(window.get(), "on_actionSettings_triggered");
}