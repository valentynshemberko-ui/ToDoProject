#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <optional>
#include <vector>
#include <string>
#include "../databaseManager/taskRepository.h"
#include "../logger/globalLogger.h"
#include "task.h"
#include "ITaskObserver.h"
#include <QWidget>

using namespace std;

class TaskManager {
private:
    TaskRepository& repo;
    string currentUser;
    vector<Task> cachedTasks;
    vector<ITaskObserver*> observers;

public:
    explicit TaskManager(TaskRepository& repository);

    void setCurrentUser(const ::string& username);

    bool addTask(QWidget* parent, const Task& task);
    bool removeTask(QWidget* parent, const ::string& title);
    bool markCompleted(QWidget* parent, const ::string& title);
    bool updateTask(QWidget* parent, const Task& task);
    vector<Task> tasksForToday(bool includeCompleted = true);
    vector<Task> loadTasks();
    void addObserver(ITaskObserver* observer);
    void removeObserver(ITaskObserver* observer);

private:
    void notifyObservers();
};

#endif //TASKMANAGER_H
