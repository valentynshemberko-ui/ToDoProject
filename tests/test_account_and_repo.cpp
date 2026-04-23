
#include <gtest/gtest.h>
#include <optional>
#include <string>
#include "../databaseManager/databaseManager.h"
#include "../databaseManager/accountRepository.h"
#include "../accounts/account.h"

TEST(AccountModel, SerializeDeserializeRoundtrip) {
    Account a("denys", 123456u);
    std::string s = a.serialize();
    Account b = Account::deserialize(s);
    EXPECT_EQ(b.getUsername(), "denys");
    EXPECT_EQ(b.getPasswordHash(), 123456u);
}

struct AccountRepoFixture : public ::testing::Test {
    DatabaseManager db = DatabaseManager(":memory:");
    std::unique_ptr<AccountRepository> repo;

    void SetUp() override {
        ASSERT_TRUE(db.open());
        repo = std::make_unique<AccountRepository>(db);
        repo->initTable();
    }
};

TEST_F(AccountRepoFixture, AddAndExists) {
    EXPECT_FALSE(repo->accountExists("a"));
    EXPECT_TRUE(repo->addAccount("a", 111u));
    EXPECT_TRUE(repo->accountExists("a"));
}

TEST_F(AccountRepoFixture, GetAccount) {
    repo->addAccount("b", 222u);
    auto acc = repo->getAccount("b");
    ASSERT_TRUE(acc.has_value());
    EXPECT_EQ(acc->getUsername(), "b");
    EXPECT_EQ(acc->getPasswordHash(), 222u);
}

TEST_F(AccountRepoFixture, GetAllAccounts) {
    repo->addAccount("u1", 1u);
    repo->addAccount("u2", 2u);
    auto all = repo->getAllAccounts();
    EXPECT_GE(all.size(), 2u);
}

TEST_F(AccountRepoFixture, UpdateUsernameAndPassword) {
    ASSERT_TRUE(repo->addAccount("old", 10u));
    // update username only
    EXPECT_TRUE(repo->updateAccount("old", "new", std::nullopt));
    EXPECT_TRUE(repo->accountExists("new"));
    EXPECT_FALSE(repo->accountExists("old"));
    // update password only
    EXPECT_TRUE(repo->updateAccount("new", "new", 999u));
    auto acc = repo->getAccount("new");
    ASSERT_TRUE(acc.has_value());
    EXPECT_EQ(acc->getPasswordHash(), 999u);
}

TEST_F(AccountRepoFixture, RemoveAccount) {
    repo->addAccount("z", 777u);
    EXPECT_TRUE(repo->removeAccount("z"));
    EXPECT_FALSE(repo->accountExists("z"));
}
