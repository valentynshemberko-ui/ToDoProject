
#include <gtest/gtest.h>
#include <QString>
#include <QDateTime>
#include "../databaseManager/databaseManager.h"
#include "../databaseManager/TaskRepository.h"
#include "../tasks/TaskManager.h"
#include "../tasks/task.h"

struct TaskManagerFx : public ::testing::Test {
    DatabaseManager db = DatabaseManager(":memory:");
    std::unique_ptr<TaskRepository> repo;
    std::unique_ptr<TaskManager> mgr;

    void SetUp() override {
        ASSERT_TRUE(db.open());
        repo = std::make_unique<TaskRepository>(db);
        mgr  = std::make_unique<TaskManager>(*repo);
        mgr->setCurrentUser("denys");
    }
};

TEST_F(TaskManagerFx, AddLoadAndComplete) {
    Task t("Build", "cmake", QDateTime::currentDateTime(), "High", false);
    EXPECT_TRUE(mgr->addTask(nullptr, t));
    auto all = mgr->loadTasks();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].getTitle(), "Build");

    EXPECT_TRUE(mgr->markCompleted(nullptr, "Build"));
    auto after = mgr->loadTasks();
    ASSERT_EQ(after.size(), 1u);
    EXPECT_TRUE(after[0].isCompleted());
}

TEST_F(TaskManagerFx, UpdateAndDelete) {
    Task t("Edit", "ui", QDateTime::currentDateTime(), "Low", false);
    ASSERT_TRUE(mgr->addTask(nullptr, t));
    t.setPriority("High");
    t.setDescription("ui/ux");
    EXPECT_TRUE(mgr->updateTask(nullptr, t));
    auto ref = mgr->loadTasks();
    ASSERT_EQ(ref.size(), 1u);
    EXPECT_EQ(ref[0].getPriority(), "High");
    EXPECT_EQ(ref[0].getDescription(), "ui/ux");

    EXPECT_TRUE(mgr->removeTask(nullptr, "Edit"));
    EXPECT_TRUE(mgr->loadTasks().empty());
}

TEST_F(TaskManagerFx, TasksForTodayFilters) {
    Task a("Today", "", QDateTime::currentDateTime(), "M", false);
    Task b("Tomorrow", "", QDateTime::currentDateTime().addDays(1), "M", false);
    ASSERT_TRUE(mgr->addTask(nullptr, a));
    ASSERT_TRUE(mgr->addTask(nullptr, b));
    auto todayList = mgr->tasksForToday();
    ASSERT_EQ(todayList.size(), 1u);
    EXPECT_EQ(todayList[0].getTitle(), "Today");
}
