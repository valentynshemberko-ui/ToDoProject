#include "taskManager.h"
#include <QMessageBox>
#include <algorithm>
#include <QDate>
using namespace std;

TaskManager::TaskManager(TaskRepository& repository) : repo(repository) {
    logger.debug("Initializing TaskManager (Qt GUI mode)");
    repo.initTable();
}

void TaskManager::setCurrentUser(const string& username) {
    currentUser = username;
    logger.info("TaskManager setCurrentUser: " + username);
}

bool TaskManager::addTask(QWidget* parent, const Task& task) {
    if (currentUser.empty()) {
        if (qEnvironmentVariableIsSet("TEST_MODE")) {
            qDebug() << "[TEST_MODE] Settings dialog suppressed";
            return false;
        }
        QMessageBox::warning(parent, "Error", "No user logged in!");
        return false;
    }
    bool ok = repo.addTask(currentUser, task);
    if (ok){
        logger.info("Task added for user " + currentUser + ": " + task.getTitle().toStdString());
        notifyObservers();
    }
    else
        QMessageBox::critical(parent, "Error", "Failed to add task to database!");
    return ok;
}

bool TaskManager::removeTask(QWidget* parent, const string& title) {
    if (currentUser.empty()) {
        if (qEnvironmentVariableIsSet("TEST_MODE")) {
            qDebug() << "[TEST_MODE] Suppressed QMessageBox (no user logged in)";
            return false;
        } else {
            QMessageBox::warning(parent, "Error", "No user logged in!");
            return false;
        }
    }

    bool ok = repo.removeTask(currentUser, title);

    notifyObservers();

    if (!ok) {
        if (qEnvironmentVariableIsSet("TEST_MODE")) {
            qDebug() << "[TEST_MODE] Suppressed QMessageBox (remove failed)";
        } else {
            QMessageBox::critical(parent, "Error", "Failed to delete task from database!");
        }
    }
    return ok;
}

bool TaskManager::markCompleted(QWidget* parent, const string& title) {
    auto taskOpt = repo.getTaskByTitle(currentUser, title);

    if (!taskOpt.has_value()) {
        QMessageBox::warning(parent, "Error", "Task not found!");
        return false;
    }

    Task t = taskOpt.value();

    if (t.isCompleted()) {
        QMessageBox::information(parent, "Info", "Task is already completed!");
        return false;
    }

    t.setCompleted(true);

    if (!repo.updateTask(currentUser, t)) {
        if (qEnvironmentVariableIsSet("TEST_MODE")) {
            qDebug() << "[TEST_MODE] Settings dialog suppressed";
            return false;
        }
        QMessageBox::critical(parent, "Error", "Failed to update task in database!");
        return false;
    }
    return true;
}

bool TaskManager::updateTask(QWidget* parent, const Task& task) {
    if (task.getTitle().isEmpty()) {
        QMessageBox::warning(parent, "Error", "Task title cannot be empty!");
        return false;
    }

    if (!repo.updateTask(currentUser, task)) {
        if (qEnvironmentVariableIsSet("TEST_MODE")) {
            qDebug() << "[TEST_MODE] Settings dialog suppressed";
            return false;
        }
        QMessageBox::critical(parent, "Error", "Failed to update task in database!");
        return false;
    }

    notifyObservers();

    return true;
}

std::vector<Task> TaskManager::tasksForToday(bool includeCompleted)
{
    std::vector<Task> todayTasks;

    if (currentUser.empty()) {
        logger.warn("tasksForToday() called with no user logged in");
        return todayTasks;
    }

    // Отримуємо всі задачі користувача
    auto allTasks = repo.getTasksByUser(currentUser);

    QDate today = QDate::currentDate();

    for (const Task &task : allTasks) {
        QDate deadline = task.getDeadline().date();

        if (deadline == today) {
            if (includeCompleted || !task.isCompleted()) {
                todayTasks.push_back(task);
            }
        }
    }

    logger.debug("tasksForToday() returned " + std::to_string(todayTasks.size()) +
                 " tasks for user " + currentUser);

    return todayTasks;
}


vector<Task> TaskManager::loadTasks() {
    if (currentUser.empty()) return {};
    return repo.getTasksByUser(currentUser);
}

// === Реалізація патерну Observer ===
void TaskManager::addObserver(ITaskObserver* observer) {
    if (std::find(observers.begin(), observers.end(), observer) == observers.end()) {
        observers.push_back(observer);
    }
}

void TaskManager::removeObserver(ITaskObserver* observer) {
    observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
}

void TaskManager::notifyObservers() {
    for (ITaskObserver* obs : observers) {
        obs->onTasksUpdated();
    }
}