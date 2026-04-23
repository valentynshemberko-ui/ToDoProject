
#include <gtest/gtest.h>
#include "../databaseManager/databaseManager.h"
#include "../databaseManager/accountRepository.h"

struct AccNegFx : public ::testing::Test {
    DatabaseManager db = DatabaseManager(":memory:");
    std::unique_ptr<AccountRepository> repo;
    void SetUp() override {
        ASSERT_TRUE(db.open());
        repo = std::make_unique<AccountRepository>(db);
        repo->initTable();
    }
};

TEST_F(AccNegFx, DuplicateUserReturnsFalse) {
    EXPECT_TRUE(repo->addAccount("u", 1u));
    EXPECT_FALSE(repo->addAccount("u", 2u));
}

TEST_F(AccNegFx, EmptyUsernameRejected) {
    EXPECT_FALSE(repo->addAccount("", 1u));
    EXPECT_FALSE(repo->accountExists(""));
    auto acc = repo->getAccount("");
    EXPECT_FALSE(acc.has_value());
}
