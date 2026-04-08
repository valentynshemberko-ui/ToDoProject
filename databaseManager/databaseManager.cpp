#include "databaseManager.h"
#include "../logger/globalLogger.h"

using namespace std;

DatabaseManager::DatabaseManager(const string& name) : db(nullptr), dbName(name) {
        if (sqlite3_open(dbName.c_str(), &db) != SQLITE_OK) {
            logger.error("Failed to open DB: " + dbName);
        } else {
            logger.debug("Database opened successfully: " + dbName);
        }
}

bool DatabaseManager::open() {
    int rc = sqlite3_open(dbName.c_str(), &db);
    if (rc != SQLITE_OK) {
        logger.error("Failed to open database: " + string(sqlite3_errmsg(db)));
    } else {
        logger.debug("Database opened successfully: " + string(dbName.c_str()));
    }
    if (rc) {
        cerr << "Failed to open database: " << sqlite3_errmsg(db) << endl;
        logger.error("Failed to open database: " + string(sqlite3_errmsg(db)));
        return false;
    }

    return true;
}

void DatabaseManager::close() {
    if (db) sqlite3_close(db);
    db = nullptr;
}

int DatabaseManager::getChanges() const {
    return sqlite3_changes(db);
}

bool DatabaseManager::execute(const string& sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        string emsg;
        if (errMsg) {
            emsg = errMsg;
            sqlite3_free(errMsg);
        } else {
            emsg = sqlite3_errstr(rc); // додатковий текст помилки (якщо доступний)
        }
        logger.error("SQL error (rc=" + to_string(rc) + "): " + emsg);
        logger.debug("Failed SQL: " + sql);
        return false;
    }
    logger.debug("SQL executed OK: " + sql);
    return true;
}

bool DatabaseManager::prepare(const string& sql, sqlite3_stmt** stmt) {
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, stmt, nullptr);
    if (rc != SQLITE_OK) {
        logger.error("SQL prepare failed: " + string(sqlite3_errmsg(db)));
        return false;
    }
    return true;
}

sqlite3* DatabaseManager::getDB() const { return db; }
DatabaseManager::~DatabaseManager() { close(); }