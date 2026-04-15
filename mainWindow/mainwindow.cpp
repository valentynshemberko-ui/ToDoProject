#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "taskitemwidget.h"
#include "taskeditorwindow.h"
#include "../settings/settingswindow.h"
#include "../settings/appsettings.h"
#include "../windowEdit/snapPreviewWindow.h"

#include <QCloseEvent>
#include <QActionGroup>
#include <QDate>
#include <QFile>
#include <QDebug>
#include <QTimer>
#include <QSoundEffect>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QScreen>
#include <QGuiApplication>
#include <QApplication>
#include <QGraphicsDropShadowEffect>
MainWindow::MainWindow(TaskManager &manager, QWidget *parent)
    : FramelessWindow(parent),
      ui(new Ui::MainWindow),
      taskManager(manager)
{
    ui->setupUi(this);

    setupUI();
    setupTrayIcon();
    setupTimers();
    setupConnections();

    taskManager.addObserver(this);
    applySettings();
    startOrStopReminders();
    loadTasks();
    enforceAutoDelete();
}

void MainWindow::setupUI()
{
    initToolBoxSections();
    updateToolBoxTitles();
    setWindowTitle("ToDo Manager");

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    resize(screenGeometry.size());
    move(screenGeometry.topLeft());
    isMaximized = true;

    snapPreview = new SnapPreviewWindow(this);

    if (AppSettings::theme() == AppSettings::Theme::Light) {
        ui->btnMaximize->setIcon(QIcon(":/resources/icons/icons-for-window/maximize-black.png"));
        ui->btnMinimize->setIcon(QIcon(":/resources/icons/icons-for-window/window-minimize-black.png"));
        ui->btnClose->setIcon(QIcon(":/resources/icons/icons-for-window/close-black.png"));
    } else {
        ui->btnMaximize->setIcon(QIcon(":/resources/icons/icons-for-window/maximize-white.png"));
        ui->btnMinimize->setIcon(QIcon(":/resources/icons/icons-for-window/window-minimize-white.png"));
        ui->btnClose->setIcon(QIcon(":/resources/icons/icons-for-window/close-white.png"));
    }

    ui->todayList->setSelectionMode(QAbstractItemView::NoSelection);
    ui->weekList->setSelectionMode(QAbstractItemView::NoSelection);
    ui->monthList->setSelectionMode(QAbstractItemView::NoSelection);
    ui->laterList->setSelectionMode(QAbstractItemView::NoSelection);
    ui->otherList->setSelectionMode(QAbstractItemView::NoSelection);

    ui->viewFilterBox->addItems({"All Tasks", "Overdue", "Completed", "Uncompleted"});

    setupTitleBar();
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setMouseTracking(true);
    ui->centralwidget->setMouseTracking(true);
}

void MainWindow::setupTrayIcon()
{
    trayIcon = new QSystemTrayIcon(this);
    trayMenu = new QMenu(this);

    QMenu *statusMenu = new QMenu("Status", trayMenu);
    QAction *statusActive = new QAction("🟢 Active", this);
    QAction *statusBusy   = new QAction("🔴 Busy", this);
    QAction *statusAway   = new QAction("🟡 Away", this);

    statusActive->setCheckable(true);
    statusBusy->setCheckable(true);
    statusAway->setCheckable(true);

    QActionGroup *statusGroup = new QActionGroup(this);
    statusGroup->addAction(statusActive);
    statusGroup->addAction(statusBusy);
    statusGroup->addAction(statusAway);

    switch (AppSettings::userStatus()) {
        case AppSettings::UserStatus::Busy: statusBusy->setChecked(true); break;
        case AppSettings::UserStatus::Away: statusAway->setChecked(true); break;
        default: statusActive->setChecked(true); break;
    }

    statusMenu->addActions(statusGroup->actions());
    connect(statusGroup, &QActionGroup::triggered, this, &MainWindow::onStatusTriggered);

    bool dark = (AppSettings::theme() == AppSettings::Theme::Dark);

    QAction *showAction     = new QAction(QIcon(dark ? ":/resources/icons/icons-for-tray/home-white.png"     : ":/resources/icons/icons-for-tray/home-black.png"),     "Show ToDo Manager", this);
    QAction *addTaskAction  = new QAction(QIcon(dark ? ":/resources/icons/icons-for-tray/add-white.png"      : ":/resources/icons/icons-for-tray/add-black.png"),      "Add task", this);
    QAction *todayAction    = new QAction(QIcon(dark ? ":/resources/icons/icons-for-tray/today-white.png"    : ":/resources/icons/icons-for-tray/today-black.png"),    "Today's tasks", this);
    QAction *settingsAction = new QAction(QIcon(dark ? ":/resources/icons/icons-for-tray/settings-white.png" : ":/resources/icons/icons-for-tray/settings-black.png"), "Settings", this);
    QAction *themeAction    = new QAction(QIcon(dark ? ":/resources/icons/icons-for-tray/theme-white.png"    : ":/resources/icons/icons-for-tray/theme-black.png"),    "Switch theme", this);
    QAction *quitAction     = new QAction(QIcon(dark ? ":/resources/icons/icons-for-tray/exit-white.png"     : ":/resources/icons/icons-for-tray/exit-black.png"),     "Exit", this);

    trayMenu->addMenu(statusMenu);
    trayMenu->addSeparator();
    trayMenu->addAction(showAction);
    trayMenu->addAction(addTaskAction);
    trayMenu->addAction(todayAction);
    trayMenu->addSeparator();
    trayMenu->addAction(settingsAction);
    trayMenu->addAction(themeAction);
    trayMenu->addSeparator();
    trayMenu->addAction(quitAction);

    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();

    applyTrayTheme();
    updateTrayTooltip();

    connect(showAction, &QAction::triggered, this, [this]() {
        this->showNormal();
        this->activateWindow();
    });
    connect(addTaskAction, &QAction::triggered, this, &MainWindow::openTaskEditor);
    connect(todayAction, &QAction::triggered, this, [this]() {
        ui->toolBox->setCurrentIndex(0);
        this->showNormal();
        this->activateWindow();
    });
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onSettingsClicked);
    connect(themeAction, &QAction::triggered, this, &MainWindow::onThemeButtonClicked);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    connect(trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            if (isHidden()) showNormal();
            else hide();
        }
    });
}

void MainWindow::setupTimers()
{
    QTimer *trayUpdateTimer = new QTimer(this);
    connect(trayUpdateTimer, &QTimer::timeout, this, &MainWindow::updateTrayTooltip);
    trayUpdateTimer->start(60000);

    reminderTimer = new QTimer(this);
    connect(reminderTimer, &QTimer::timeout, this, &MainWindow::checkRemindersTick);

    audioOut = new QAudioOutput(this);
    reminderPlayer = new QMediaPlayer(this);
    reminderPlayer->setAudioOutput(audioOut);

    autoDeleteTimer = new QTimer(this);
    connect(autoDeleteTimer, &QTimer::timeout, this, [this]() {
        enforceAutoDelete();
        loadTasks();
    });
    autoDeleteTimer->start(60 * 1000);
}

void MainWindow::setupConnections()
{
    connect(this, &MainWindow::themeChanged, this, [this](AppSettings::Theme) {
        applyTrayTheme();
        updateTrayTooltip();
    });

    connect(ui->quickAddButton, &QPushButton::clicked, this, &MainWindow::addQuickTask);
    connect(ui->addTaskButton,  &QPushButton::clicked, this, &MainWindow::openTaskEditor);
    connect(ui->settingsButton, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);
    connect(ui->themeButton,    &QPushButton::clicked, this, &MainWindow::onThemeButtonClicked);
    connect(this, &FramelessWindow::windowMaximizedChanged, this, &MainWindow::updateMaximizeIcon);

    connect(ui->viewFilterBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onFilterChanged);
}

void MainWindow::onStatusTriggered(QAction *act)
{
    if (act->text().contains("Active"))
        AppSettings::setUserStatus(AppSettings::UserStatus::Active);
    else if (act->text().contains("Busy"))
        AppSettings::setUserStatus(AppSettings::UserStatus::Busy);
    else
        AppSettings::setUserStatus(AppSettings::UserStatus::Away);

    trayIcon->showMessage("Status changed to:",
        AppSettings::userStatusEmoji() + " Now you are " + AppSettings::userStatusName(),
        QSystemTrayIcon::Information, 2500);

    if (AppSettings::userStatus() == AppSettings::UserStatus::Busy)
        reminderTimer->stop();
    else
        startOrStopReminders();

    updateTrayTooltip();
}

MainWindow::~MainWindow()
{
    if (snapPreview) {
        snapPreview->hidePreview();
        snapPreview->deleteLater();
    }
    taskManager.addObserver(this);
    delete ui;
}

void MainWindow::onTasksUpdated() {
    loadTasks();
}

void MainWindow::applyTrayTheme()
{
    bool dark = (AppSettings::theme() == AppSettings::Theme::Dark);

    trayIcon->setIcon(QIcon(dark
        ? ":/resources/icons/icons-for-tray/to-do-white.png"
        : ":/resources/icons/icons-for-tray/to-do-black.png"));

    auto update = [dark](QAction *a, const QString &name) {
        if (!a) return;
        a->setIcon(QIcon(dark
            ? QString(":/resources/icons/icons-for-tray/%1-white.png").arg(name)
            : QString(":/resources/icons/icons-for-tray/%1-black.png").arg(name)));
    };

    update(trayMenu->actions().at(2), "home");
    update(trayMenu->actions().at(3), "add");
    update(trayMenu->actions().at(4), "today");
    update(trayMenu->actions().at(6), "settings");
    update(trayMenu->actions().at(7), "theme");
    update(trayMenu->actions().at(9), "exit");
}

void MainWindow::updateTrayTooltip()
{
    int todayCount = taskManager.tasksForToday(false).size();
    QString emoji = AppSettings::userStatusEmoji();
    QString status = AppSettings::userStatusName();

    QString themeStr = (AppSettings::theme() == AppSettings::Theme::Dark)
        ? "🌙 Black theme" : "☀️ Light theme";

    trayIcon->setToolTip(QString("%1 %2 • %3\n📅 %4 tasks%5 today")
                         .arg(emoji)
                         .arg(status)
                         .arg(themeStr)
                         .arg(QString::number(todayCount))
                         .arg(todayCount == 1 ? "а" : (todayCount < 5 ? "і" : "")));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (trayIcon && trayIcon->isVisible()) {
        hide();
        event->ignore();
    } else {
        QMainWindow::closeEvent(event);
    }
}

void MainWindow::updateMaximizeIcon(bool maxed) {
    bool isLight = (AppSettings::theme() == AppSettings::Theme::Light);

    QString path;
    if (maxed)
        path = isLight
            ? ":/resources/icons/icons-for-window/minimize-black.png"
            : ":/resources/icons/icons-for-window/minimize-white.png";
    else
        path = isLight
            ? ":/resources/icons/icons-for-window/maximize-black.png"
            : ":/resources/icons/icons-for-window/maximize-white.png";

    ui->btnMaximize->setIcon(QIcon(path));
}

void MainWindow::onSettingsClicked() {
    if (qEnvironmentVariableIsSet("TEST_MODE")) {
        qDebug() << "[TEST_MODE] Suppressed Settings dialog";
        return;
    }

    SettingsWindow dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        applySettings();
        enforceAutoDelete();
        startOrStopReminders();
        loadTasks();
    }
}

void MainWindow::onThemeButtonClicked() {
    using Theme = AppSettings::Theme;
    Theme current = AppSettings::theme();
    Theme next = (current == Theme::Light) ? Theme::Dark : Theme::Light;

    ui->themeButton->setIconSize(QSize(32, 32));

    if (AppSettings::theme() == AppSettings::Theme::Light){
        ui->themeButton->setIcon(QIcon(":/resources/icons/dark-theme.png"));
        ui->btnMinimize->setIcon(QIcon(":/resources/icons/icons-for-window/window-minimize-white.png"));
        ui->btnClose->setIcon(QIcon(":/resources/icons/icons-for-window/close-white.png"));
    }
    else {
        ui->themeButton->setIcon(QIcon(":/resources/icons/light-theme.png"));
        ui->btnMinimize->setIcon(QIcon(":/resources/icons/icons-for-window/window-minimize-black.png"));
        ui->btnClose->setIcon(QIcon(":/resources/icons/icons-for-window/close-black.png"));
    }

    AppSettings::setTheme(next);
    emit themeChanged(next);
    applySettings();
    loadTasks();
    applyTrayTheme();

    updateMaximizeIcon(isMaximized);

    QString mode = (next == Theme::Light) ? "Light" : "Dark";
    ui->statusbar->showMessage("Theme switched to " + mode, 3000);
}

void MainWindow::applySettings() {
    QFile f;
    if (AppSettings::theme() == AppSettings::Theme::Dark)
        f.setFileName(":/styles/main_dark.qss");
    else
        f.setFileName(":/styles/main.qss");

    if (AppSettings::theme() == AppSettings::Theme::Light)
        ui->settingsButton->setIcon(QIcon(":/resources/icons/setting-dark.png"));
    else
        ui->settingsButton->setIcon(QIcon(":/resources/icons/setting-white.png"));
    ui->settingsButton->setIconSize(QSize(32, 32));

    if (AppSettings::theme() == AppSettings::Theme::Light)
        ui->themeButton->setIcon(QIcon(":/resources/icons/dark-theme.png"));
    else
        ui->themeButton->setIcon(QIcon(":/resources/icons/light-theme.png"));
    ui->themeButton->setIconSize(QSize(32, 32));

    if (f.open(QFile::ReadOnly)) {
        qApp->setStyleSheet(QLatin1String(f.readAll()));
        f.close();
    }
}

void MainWindow::startOrStopReminders() {
    remindedKeys.clear();
    if (AppSettings::reminderEnabled()) {
        reminderTimer->start(60 * 1000);
        const QString path = AppSettings::reminderSound();
        if (!path.isEmpty() && QFileInfo::exists(path)) {
            reminderPlayer->setSource(QUrl::fromLocalFile(path));
        } else {
            reminderPlayer->setSource(QUrl());
        }
    } else {
        reminderTimer->stop();
    }
}

void MainWindow::checkRemindersTick() {
    if (!AppSettings::reminderEnabled())
        return;

    const int remindBeforeMin = AppSettings::reminderMinutes();
    const QDateTime now = QDateTime::currentDateTime();

    auto tasks = taskManager.loadTasks();
    for (const auto &task : tasks) {
        processSingleTaskReminder(task, now, remindBeforeMin);
    }
}

void MainWindow::processSingleTaskReminder(const Task& task, const QDateTime& now, int remindBeforeMin) {
    if (task.isCompleted())
        return;

    const QDateTime deadline = task.getDeadline();
    if (!deadline.isValid())
        return;

    const qint64 minutesLeft = now.secsTo(deadline) / 60;
    if (minutesLeft < 0)
        return;

    const QString key = task.getTitle() + "|" + deadline.toString(Qt::ISODate);

    if (minutesLeft <= remindBeforeMin && !remindedKeys.contains(key)) {
        remindedKeys.insert(key);

        if (reminderPlayer && !reminderPlayer->source().isEmpty()) {
            reminderPlayer->stop();
            reminderPlayer->play();
        }

        QTimer::singleShot(300, this, [=]() {
            const QString msg = QString(
                    "Task: <b>%1</b><br>"
                    "Deadline: %2<br>"
                    "Time left: %3 min")
                    .arg(task.getTitle())
                    .arg(deadline.toString("dd.MM.yyyy hh:mm"))
                    .arg(QString::number(minutesLeft));

            FramelessMessageBox::information(this, "Reminder", msg);
        });
    } else if (minutesLeft > remindBeforeMin) {
        remindedKeys.remove(key);
    }
}

void MainWindow::enforceAutoDelete() {
    if (!AppSettings::autoDelete()) return;

    const auto mode = AppSettings::deletePeriod();
    const QDateTime now = QDateTime::currentDateTime();
    const auto tasks = taskManager.loadTasks();

    for (const auto &t : tasks) {
        if (shouldAutoDeleteTask(t, now, mode)) {
            taskManager.removeTask(this, t.getTitle().toStdString());
        }
    }
}

bool MainWindow::shouldAutoDeleteTask(const Task& t, const QDateTime& now, AppSettings::AutoDeletePeriod mode) const {
    if (!t.isCompleted())
        return false;

    const QDateTime dl = t.getDeadline();

    switch (mode) {
        case AppSettings::AutoDeletePeriod::Immediately:
            return true;
        case AppSettings::AutoDeletePeriod::After1Day:
            return dl.isValid() && dl.addDays(1) <= now;
        case AppSettings::AutoDeletePeriod::After1Week:
            return dl.isValid() && dl.addDays(7) <= now;
        case AppSettings::AutoDeletePeriod::AfterDeadline:
            return dl.isValid() && dl <= now;
        default:
            return false;
    }
}

void MainWindow::clearAllLists() {
    auto clear = [](QListWidget *list) {
        while (list->count() > 0) {
            QListWidgetItem *item = list->takeItem(0);
            QWidget *w = list->itemWidget(item);
            if (w) w->deleteLater();
            delete item;
        }
    };
    clear(ui->todayList);
    clear(ui->weekList);
    clear(ui->monthList);
    clear(ui->laterList);
    clear(ui->otherList);
}

void MainWindow::loadTasks() {
    clearAllLists();

    QDateTime today = QDateTime::currentDateTime();
    auto tasks = taskManager.loadTasks();
    int filter = ui->viewFilterBox->currentIndex();

    for (const auto &t : tasks) {
        bool overdue = (t.getDeadline() < today && !t.isCompleted());
        bool completed = t.isCompleted();
        bool uncompleted = !t.isCompleted();

        if (filter == 1 && !overdue) continue;
        if (filter == 2 && !completed) continue;
        if (filter == 3 && !uncompleted) continue;

        addTaskToToolBox(t);
    }
    updateToolBoxTitles();
}

void MainWindow::addTaskToToolBox(const Task &task) {
    QDateTime today = QDateTime::currentDateTime();
    QDateTime deadline = task.getDeadline();
    QListWidget *target = ui->otherList;

    if (deadline.date() < today.date()) target = ui->otherList;
    else if (deadline.date() == today.date()) target = ui->todayList;
    else if (deadline.date() <= today.date().addDays(7)) target = ui->weekList;
    else if (deadline.date() <= today.date().addMonths(1)) target = ui->monthList;
    else if (deadline.date().isValid()) target = ui->laterList;

    auto *item = new QListWidgetItem(target);
    auto *taskWidget = new TaskItemWidget(task);
    taskWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    taskWidget->setMinimumHeight(68);
    item->setSizeHint(QSize(0, taskWidget->minimumHeight() + 8));

    target->addItem(item);
    target->setItemWidget(item, taskWidget);

    connect(taskWidget, &TaskItemWidget::requestEdit,    this, &MainWindow::handleTaskEdit);
    connect(taskWidget, &TaskItemWidget::requestDone,    this, &MainWindow::handleTaskDone);
    connect(taskWidget, &TaskItemWidget::requestDetails, this, &MainWindow::handleTaskDetails);
    connect(taskWidget, &TaskItemWidget::requestDelete,  this, &MainWindow::handleTaskDelete);

    updateToolBoxTitleFor(target);
    taskWidget->setStyleSheet(qApp->styleSheet());
}

void MainWindow::updateToolBoxTitles() {
    for (const auto &s : sections) {
        if (!s.page || !s.list || s.index < 0) continue;
        ui->toolBox->setItemText(s.index,
                                 QString("%1 (%2)").arg(s.baseTitle).arg(QString::number(s.list->count())));
    }
}

void MainWindow::updateToolBoxTitleFor(QListWidget *list) {
    for (const auto &s : sections) {
        if (s.list == list && s.index >= 0) {
            ui->toolBox->setItemText(s.index,
                                     QString("%1 (%2)").arg(s.baseTitle).arg(QString::number(list->count())));
            break;
        }
    }
}

void MainWindow::addQuickTask() {
    QString text = ui->taskInput->text().trimmed();
    if (text.isEmpty()) {
        FramelessMessageBox::warning(this, "Warning", "Please enter a task title!");
        return;
    }
    QDateTime defDeadline = AppSettings::computeQuickAddDeadline();
    Task quickTask(text, "Quick added task", defDeadline, "Medium", false);
    if (taskManager.addTask(this, quickTask)) {
        enforceAutoDelete();
        ui->taskInput->clear();
        loadTasks();
    }
}

void MainWindow::openTaskEditor() {
    if (qEnvironmentVariableIsSet("TEST_MODE")) {
        qDebug() << "[TEST_MODE] Suppressed TaskEditor dialog";
        return;
    }

    TaskEditorWindow editor(this);
    if (editor.exec() == QDialog::Accepted) {
        Task t = editor.getTask();
        if (taskManager.addTask(this, t)) {
            FramelessMessageBox::information(this, "Success", "Task added successfully!");
            enforceAutoDelete();
            loadTasks();
        } else {
            FramelessMessageBox::critical(this, "Error", "Failed to add task!");
        }
    }
}

void MainWindow::handleTaskEdit(const Task &task) {
    TaskEditorWindow ed(this);
    ed.setTask(task);
    if (ed.exec() == QDialog::Accepted) {
        Task upd = ed.getTask();
        if (taskManager.updateTask(this, upd)) {
            FramelessMessageBox::information(this, "Updated", "Task updated successfully!");
            enforceAutoDelete();
            loadTasks();
        }
    }
}

void MainWindow::handleTaskDone(const Task &task) {
    if (task.isCompleted()) {
        FramelessMessageBox::information(this, "Info", "Task already completed.");
        return;
    }
    if (!taskManager.markCompleted(this, task.getTitle().toStdString())) {
        FramelessMessageBox::warning(this, "Error", "Failed to update status!");

        return;
    }
    enforceAutoDelete();
    loadTasks();
}

void MainWindow::handleTaskDelete(const Task &task) {
    if (FramelessMessageBox::question(this, "Delete", "Delete \"" + task.getTitle() + "\"?")
        == QMessageBox::Yes) {
        if (taskManager.removeTask(this, task.getTitle().toStdString()))
            loadTasks();
    }
}

void MainWindow::handleTaskDetails(const Task &task) {
    QString info = QString("Title: %1\nDescription: %2\nDeadline: %3\nPriority: %4\nCompleted: %5")
            .arg(task.getTitle())
            .arg(task.getDescription())
            .arg(task.getDeadline().toString("dd.MM.yyyy hh:mm"))
            .arg(task.getPriority())
            .arg(task.isCompleted() ? "Yes" : "No");
    FramelessMessageBox::information(this, "Details", info);
}

void MainWindow::onFilterChanged(int) { loadTasks(); }

void MainWindow::initToolBoxSections() {
    sections.clear();
    const QVector<QPair<QWidget*, QListWidget*>> pairs = {
        {ui->todayPage,  ui->todayList},
        {ui->weekPage,   ui->weekList},
        {ui->monthPage,  ui->monthList},
        {ui->laterPage,  ui->laterList},
        {ui->otherPage,  ui->otherList}
    };

    for (const auto &p : pairs) {
        Section s;
        s.page = p.first;
        s.list = p.second;
        s.index = ui->toolBox->indexOf(s.page);
        if (s.index < 0) continue;
        s.baseTitle = ui->toolBox->itemText(s.index).trimmed();
        if (s.baseTitle.isEmpty()) {
            if (s.page == ui->todayPage) s.baseTitle = "Today";
            else if (s.page == ui->weekPage) s.baseTitle = "This Week";
            else if (s.page == ui->monthPage) s.baseTitle = "This Month";
            else if (s.page == ui->laterPage) s.baseTitle = "Later";
            else if (s.page == ui->otherPage) s.baseTitle = "Other";
        }
        sections.push_back(s);
    }
}

void MainWindow::setupTitleBar() {
    ui->titleBar->setMinimumHeight(36);
    ui->titleBar->setMaximumHeight(36);

    connect(ui->btnClose,    &QPushButton::clicked, this, &QWidget::close);
    connect(ui->btnMinimize, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(ui->btnMaximize, &QPushButton::clicked, this, &MainWindow::toggleMaximizeRestore);

    if (auto *v = qobject_cast<QVBoxLayout*>(ui->centralwidget->layout())) {
        v->setContentsMargins(0,0,0,0);
        v->setSpacing(0);
    }
}