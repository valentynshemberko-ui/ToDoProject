#ifndef FRIEND_MAINWINDOW_H
#define FRIEND_MAINWINDOW_H
#include "../mainWindow/mainwindow.h"

class TestFriend_MainWindow : public MainWindow {
public:
    explicit TestFriend_MainWindow(TaskManager &manager, QWidget *parent = nullptr)
      : MainWindow(manager, parent) {}

    using MainWindow::updateTrayTooltip;
    using MainWindow::applyTrayTheme;
    using MainWindow::loadTasks;
    using MainWindow::updateMaximizeIcon;
};
#endif //FRIEND_MAINWINDOW_H
