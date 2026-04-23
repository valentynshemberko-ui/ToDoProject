
#include <gtest/gtest.h>
#include "../databaseManager/databaseManager.h"

TEST(DatabaseManagerNeg, PrepareInvalidSQLFailsGracefully) {
    DatabaseManager db(":memory:");
    ASSERT_TRUE(db.open());
    sqlite3_stmt* s = nullptr;
    EXPECT_FALSE(db.prepare("THIS IS NOT SQL", &s));
    EXPECT_EQ(s, nullptr);
}
