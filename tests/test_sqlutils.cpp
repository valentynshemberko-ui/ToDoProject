
#include <gtest/gtest.h>
#include "../databaseManager/SQLUtilities/SQLUtils.h"

TEST(SQLUtils, EscapeQuotes) {
    std::string input = "O'Brien's book";
    std::string out = escapeSQL(input);
    EXPECT_EQ(out, "O''Brien''s book");
}

TEST(SQLUtils, NoChangeForSafeString) {
    std::string input = "alpha beta gamma";
    std::string out = escapeSQL(input);
    EXPECT_EQ(out, input);
}

TEST(SQLUtils, EmptyString) {
    std::string input = "";
    std::string out = escapeSQL(input);
    EXPECT_TRUE(out.empty());
}
