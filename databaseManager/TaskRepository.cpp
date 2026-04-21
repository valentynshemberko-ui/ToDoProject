#include "taskRepository.h"
#include "SQLUtilities/SQLUtils.h"
#include <sstream>
#include <iomanip>

using namespace std;

TaskRepository::TaskRepository(DatabaseManager& database) : db(database) {}

void TaskRepository::initTable() {
    string sql =
        "CREATE TABLE IF NOT EXISTS tasks ("
        "username TEXT NOT NULL, "
        "title TEXT NOT NULL, "
        "description TEXT, "
        "deadline TEXT, "
        "priority TEXT, "
        "completed INTEGER DEFAULT 0, "
        "PRIMARY KEY (username, title), "
        "FOREIGN KEY(username) REFERENCES accounts(username) ON DELETE CASCADE"
        ");";

    if (!db.execute(sql))
        logger.error("Failed to create table 'tasks'. See SQL error above.");
    else
        logger.info("Table 'tasks' ensured.");
}

bool TaskRepository::addTask(const string& username, const Task& task) {
    string checkSql =
        "SELECT COUNT(*) FROM tasks WHERE username='" + escapeSQL(username) +
        "' AND title='" + escapeSQL(task.getTitle().toStdString()) + "';";

    sqlite3_stmt* stmt = nullptr;
    if (!db.prepare(checkSql, &stmt)) {
        logger.error("addTask: failed to prepare SELECT");
        return false;
    }

    bool exists = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int count = sqlite3_column_int(stmt, 0);
        exists = (count > 0);
    }
    sqlite3_finalize(stmt);

    if (exists) {
        logger.warn("Task already exists: " + task.getTitle().toStdString());
        return false;
    }

    string sql =
        "INSERT INTO tasks (username, title, description, deadline, priority, completed) VALUES ('" +
        escapeSQL(username) + "', '" + escapeSQL(task.getTitle().toStdString()) + "', '" +
        escapeSQL(task.getDescription().toStdString()) + "', '" +
        task.getDeadline().toString(Qt::ISODate).toStdString() + "', '" +
        escapeSQL(task.getPriority().toStdString()) + "', " +
        (task.isCompleted() ? "1" : "0") + ");";

    bool ok = db.execute(sql);
    if (ok)
        logger.info("Task added: " + task.getTitle().toStdString());
    else
        logger.error("Failed to add task for " + username);

    return ok;
}


bool TaskRepository::removeTask(const string& username, const string& title) {
    string sql = "DELETE FROM tasks WHERE username='" + escapeSQL(username) +
                 "' AND title='" + escapeSQL(title) + "';";
    bool ok = db.execute(sql);

    int changes = db.getChanges();
    if (changes == 0) {
        logger.warn("No task found to remove: " + title);
        return false;
    }

    if (ok) logger.info("Task removed: " + title);
    else logger.warn("Failed to remove task: " + title);
    return ok;
}

bool TaskRepository::updateTask(const string& username, const Task& task) {
    string sql =
        "UPDATE tasks SET "
        "description='" + escapeSQL(task.getDescription().toStdString()) + "', "
        "deadline='" + task.getDeadline().toString(Qt::ISODate).toStdString() + "', "
        "priority='" + escapeSQL(task.getPriority().toStdString()) + "', "
        "completed=" + to_string(task.isCompleted() ? 1 : 0) +
        " WHERE username='" + escapeSQL(username) +
        "' AND title='" + escapeSQL(task.getTitle().toStdString()) + "';";

    bool ok = db.execute(sql);
    int changed = db.getChanges();

    if (!ok || changed == 0) {
        logger.warn("No task found to update: " + task.getTitle().toStdString());
        return false;
    }

    logger.info("Task updated: " + task.getTitle().toStdString());
    return true;
}

vector<Task> TaskRepository::getTasksByUser(const string& username) {
    vector<Task> tasks;
    string sql =
        "SELECT title, description, deadline, priority, completed FROM tasks WHERE username='" +
        escapeSQL(username) + "';";

    sqlite3_stmt* stmt = nullptr;
    if (!db.prepare(sql, &stmt)) {
        logger.error("Failed to prepare SELECT for user tasks: " + username);
        return tasks;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        string title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        string desc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        string deadlineStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        string priority = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        bool completed = sqlite3_column_int(stmt, 4);

        QDateTime date = QDateTime::fromString(QString::fromStdString(deadlineStr), Qt::ISODate);
        tasks.emplace_back(QString::fromStdString(title),
                           QString::fromStdString(desc),
                           date,
                           QString::fromStdString(priority),
                           completed);
    }

    sqlite3_finalize(stmt);
    return tasks;
}

optional<Task> TaskRepository::getTaskByTitle(const string& username, const string& title) {
    string sql = "SELECT title, description, deadline, priority, completed "
                      "FROM tasks WHERE username='" + escapeSQL(username) +
                      "' AND title='" + escapeSQL(title) + "' LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;
    if (!db.prepare(sql, &stmt)) return nullopt;

    optional<Task> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        QString qTitle = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        QString qDesc = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        QString qDeadline = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        QString qPriority = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
        bool completed = sqlite3_column_int(stmt, 4);

        QDateTime date = QDateTime::fromString(qDeadline, Qt::ISODate);
        result = Task(qTitle, qDesc, date, qPriority, completed);
    }

    sqlite3_finalize(stmt);
    return result;
}

