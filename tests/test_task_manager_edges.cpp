
#include <gtest/gtest.h>
#include <QDateTime>
#include "../databaseManager/databaseManager.h"
#include "../databaseManager/TaskRepository.h"
#include "../tasks/TaskManager.h"
#include "../tasks/task.h"

struct TMEdgeFx : public ::testing::Test {
    DatabaseManager db = DatabaseManager(":memory:");
    std::unique_ptr<TaskRepository> repo;
    std::unique_ptr<TaskManager> mgr;
    void SetUp() override {
        ASSERT_TRUE(db.open());
        repo = std::make_unique<TaskRepository>(db);
        repo->initTable();
        mgr = std::make_unique<TaskManager>(*repo);
    }
};

TEST_F(TMEdgeFx, ActionsWithoutUserReturnEmptyOrFalse) {
    Task t("A","", QDateTime::currentDateTime(), "Low", false);
    EXPECT_FALSE(mgr->addTask(nullptr, t));
    EXPECT_FALSE(mgr->removeTask(nullptr, "A"));
    EXPECT_TRUE(mgr->loadTasks().empty());
}

TEST_F(TMEdgeFx, UpdateAndRemoveNonExistingReturnFalse) {
    mgr->setCurrentUser("denys");
    Task t("NotExists","", QDateTime::currentDateTime(), "Low", false);
    EXPECT_FALSE(mgr->updateTask(nullptr, t));
    EXPECT_FALSE(mgr->removeTask(nullptr, "NotExists"));
}
