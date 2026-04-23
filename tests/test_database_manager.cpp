
#include <gtest/gtest.h>
#include "../databaseManager/databaseManager.h"

TEST(DatabaseManager, OpenCloseInMemory) {
    DatabaseManager db(":memory:");
    EXPECT_TRUE(db.open());
    EXPECT_NE(db.getDB(), nullptr);
    db.close();
    db.close();
}

TEST(DatabaseManager, ExecuteAndPrepare) {
    DatabaseManager db(":memory:");
    ASSERT_TRUE(db.open());
    ASSERT_TRUE(db.execute("CREATE TABLE t(id INTEGER PRIMARY KEY, name TEXT);"));
    ASSERT_TRUE(db.execute("INSERT INTO t(name) VALUES ('a');"));
    sqlite3_stmt* stmt = nullptr;
    ASSERT_TRUE(db.prepare("SELECT name FROM t WHERE id=1;", &stmt));
    ASSERT_NE(stmt, nullptr);
    sqlite3_finalize(stmt);
}
