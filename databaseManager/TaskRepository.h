#ifndef TASKREPOSITORY_H
#define TASKREPOSITORY_H
#include <string>
#include <vector>
#include <optional>
#include <sqlite3.h>
#include "../databaseManager/databaseManager.h"
#include "../logger/globalLogger.h"
#include "../tasks/task.h"
using namespace std;

class TaskRepository {
private:
    DatabaseManager& db;

public:
    explicit TaskRepository(DatabaseManager& database);

    void initTable();
    bool addTask(const string& username, const Task& task);
    bool removeTask(const string& username, const string& title);
    bool updateTask(const string& username, const Task& task);
    vector<Task> getTasksByUser(const string& username);
    optional<Task> getTaskByTitle(const string& username, const string& title);
};


#endif //TASKREPOSITORY_H
