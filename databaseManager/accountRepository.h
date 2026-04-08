#ifndef ACCOUNTREPOSITORY_H
#define ACCOUNTREPOSITORY_H

#include "databaseManager.h"
#include <string>
#include <vector>
#include <optional>
#include "../accounts/authManager.h"
#include "../accounts/account.h"

using namespace std;

class AccountRepository {
private:
    DatabaseManager& db;

public:
    explicit AccountRepository(DatabaseManager& database);

    void initTable();

    bool addAccount(const string& username, size_t passwordHash);
    bool removeAccount(const string& username);
    bool updateAccount(const string& username, const string& newUsername,
                       optional<size_t> newPasswordHash);

    optional<Account> getAccount(const string& username);
    vector<Account> getAllAccounts();
    bool accountExists(const string& username);
};

#endif // ACCOUNTREPOSITORY_H
