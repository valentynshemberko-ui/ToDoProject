
#include <gtest/gtest.h>
#include <QDateTime>
#include "../databaseManager/databaseManager.h"
#include "../databaseManager/TaskRepository.h"
#include "../tasks/task.h"

struct TaskNegFx : public ::testing::Test {
    DatabaseManager db = DatabaseManager(":memory:");
    std::unique_ptr<TaskRepository> repo;
    void SetUp() override {
        ASSERT_TRUE(db.open());
        repo = std::make_unique<TaskRepository>(db);
        repo->initTable();
    }
};

TEST_F(TaskNegFx, GetMissingAndDeleteMissing) {
    auto t = repo->getTaskByTitle("user", "missing");
    EXPECT_FALSE(t.has_value());
    EXPECT_FALSE(repo->removeTask("user", "missing"));
}

TEST_F(TaskNegFx, SqlInjectionSafe) {
    Task tricky("x'; DROP TABLE tasks; --", "d", QDateTime::currentDateTime(), "M", false);
    EXPECT_TRUE(repo->addTask("u", tricky));
    auto list = repo->getTasksByUser("u");
    ASSERT_EQ(list.size(), 1u);
}
