#include "AccountRepository.h"
#include "../logger/globalLogger.h"
#include "SQLUtilities/SQLUtils.h"
#include <sstream>
#include <algorithm>
using namespace std;

AccountRepository::AccountRepository(DatabaseManager& database) : db(database) {}

void AccountRepository::initTable() {
    string sql =
        "CREATE TABLE IF NOT EXISTS accounts ("
        "username TEXT PRIMARY KEY, "
        "passwordHash TEXT NOT NULL"
        ");";
    if (!db.getDB()) {
        logger.error("Database pointer is null before initTable!");
        return;
    }
    const char* dbFile = sqlite3_db_filename(db.getDB(), "main");
    logger.debug(string("AccountRepository::initTable - DB file: ") + (dbFile ? dbFile : "unknown"));

    if (!db.execute(sql)) {
        logger.error("Failed to create table 'accounts'. See previous SQL error.");
        return;
    }
    logger.info("Table 'accounts' ensured.");
}

bool AccountRepository::addAccount(const string& username, size_t passwordHash) {
    if (username.empty() ||
        all_of(username.begin(), username.end(), [](unsigned char c){ return isspace(c); })) {
        logger.warn("Attempt to add account with empty or whitespace-only username.");
        return false;
        }

    string safeUsername = escapeSQL(username);

    string sql =
        "INSERT INTO accounts (username, passwordHash) VALUES ('" +
        safeUsername + "', '" + std::to_string(passwordHash) + "');";

    bool ok = db.execute(sql);
    if (ok)
        logger.info("Account added: " + username);
    else
        logger.error("Failed to insert account: " + username);

    return ok;
}

bool AccountRepository::removeAccount(const string& username) {
    string sql = "DELETE FROM accounts WHERE username='" + username + "';";
    bool ok = db.execute(sql);
    if (ok) logger.info("Account removed: " + username);
    else logger.warn("Failed to remove account: " + username);
    return ok;
}

bool AccountRepository::updateAccount(const string& username,
                                      const string& newUsername,
                                      optional<size_t> newPasswordHash) {
    ostringstream oss;
    oss << "UPDATE accounts SET username='" << newUsername << "'";

    if (newPasswordHash.has_value())
        oss << ", passwordHash='" << to_string(*newPasswordHash) << "'";

    oss << " WHERE username='" << username << "';";

    bool ok = db.execute(oss.str());
    if (ok) logger.info("Account updated: " + username);
    else logger.error("Failed to update account: " + username);
    return ok;
}

optional<Account> AccountRepository::getAccount(const string& username) {
    if (username.empty() ||
    all_of(username.begin(), username.end(), [](unsigned char c){ return isspace(c); })) {
        return nullopt;
    }
    string sql = "SELECT username, passwordHash FROM accounts WHERE username='" + username + "';";
    sqlite3_stmt* stmt;
    if (!db.prepare(sql, &stmt)) {
        logger.error("Failed to prepare SELECT for account: " + username);
        return nullopt;
    }

    optional<Account> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        string u = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        string hashStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        size_t hashValue = 0;
        try {
            hashValue = static_cast<size_t>(stoull(hashStr));
        } catch (...) {
            logger.warn("Failed to parse password hash for user: " + u);
        }
        result = Account(u, hashValue);
    } else {
        logger.debug("No account found for username: " + username);
    }

    sqlite3_finalize(stmt);
    return result;
}

vector<Account> AccountRepository::getAllAccounts() {
    vector<Account> res;
    string sql = "SELECT username, passwordHash FROM accounts;";
    sqlite3_stmt* stmt;

    if (!db.prepare(sql, &stmt)) {
        logger.error("Failed to prepare SELECT for all accounts.");
        return res;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        string u = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        string hashStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        size_t hashValue = 0;
        try {
            hashValue = static_cast<size_t>(stoull(hashStr));
        } catch (...) {
            logger.warn("Failed to parse hash for user: " + u);
        }
        res.emplace_back(u, hashValue);
    }

    sqlite3_finalize(stmt);
    return res;
}

bool AccountRepository::accountExists(const string& username) {
    if (username.empty() ||
        all_of(username.begin(), username.end(), [](unsigned char c){ return isspace(c); })) {
        return false;
        }
    string sql = "SELECT COUNT(*) FROM accounts WHERE username=?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db.getDB(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        logger.error("accountExists: failed to prepare SQL: " + string(sqlite3_errmsg(db.getDB())));
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    bool exists = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int count = sqlite3_column_int(stmt, 0);
        exists = (count > 0);
    } else {
        logger.warn("accountExists: sqlite3_step returned unexpected result");
    }

    sqlite3_finalize(stmt);
    logger.debug("accountExists('" + username + "') = " + string(exists ? "true" : "false"));
    return exists;
}
