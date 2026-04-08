#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H
#include <sqlite3.h>
#include <string>
#include <iostream>
using namespace std;

class DatabaseManager {
private:
    sqlite3* db;
    string dbName;

public:
    DatabaseManager(const string& name= "zoo.db");

    bool open();
    void close();

    int getChanges() const;

    bool execute(const string& sql);
    bool prepare(const string& sql, sqlite3_stmt** stmt);

    sqlite3* getDB() const;
    ~DatabaseManager();
};


#endif //DATABASEMANAGER_H
