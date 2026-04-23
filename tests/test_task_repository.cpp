
#include <gtest/gtest.h>
#include <QString>
#include <QDateTime>
#include "../databaseManager/databaseManager.h"
#include "../databaseManager/TaskRepository.h"
#include "../tasks/task.h"

struct TaskRepoFixture : public ::testing::Test {
    DatabaseManager db = DatabaseManager(":memory:");
    std::unique_ptr<TaskRepository> repo;

    void SetUp() override {
        ASSERT_TRUE(db.open());
        repo = std::make_unique<TaskRepository>(db);
        repo->initTable();
    }
};

TEST_F(TaskRepoFixture, AddAndGetByUser) {
    Task t("A", "desc", QDateTime::fromString("2025-10-30T10:00:00", Qt::ISODate), "High", false);
    EXPECT_TRUE(repo->addTask("user1", t));
    auto list = repo->getTasksByUser("user1");
    ASSERT_EQ(list.size(), 1u);
    EXPECT_EQ(list[0].getTitle(), "A");
}

TEST_F(TaskRepoFixture, GetByTitleAndUpdate) {
    Task t("X", "d", QDateTime::currentDateTime(), "Low", false);
    ASSERT_TRUE(repo->addTask("u", t));
    auto opt = repo->getTaskByTitle("u", "X");
    ASSERT_TRUE(opt.has_value());
    Task updated = opt.value();
    updated.setCompleted(true);
    updated.setPriority("High");
    EXPECT_TRUE(repo->updateTask("u", updated));

    auto opt2 = repo->getTaskByTitle("u", "X");
    ASSERT_TRUE(opt2.has_value());
    EXPECT_TRUE(opt2->isCompleted());
    EXPECT_EQ(opt2->getPriority(), "High");
}

TEST_F(TaskRepoFixture, RemoveAndDelete) {
    Task t("T", "d", QDateTime::currentDateTime(), "Mid", false);
    ASSERT_TRUE(repo->addTask("u2", t));
    EXPECT_TRUE(repo->removeTask("u2", "T"));
    auto opt = repo->getTaskByTitle("u2", "T");
    EXPECT_FALSE(opt.has_value());
}
