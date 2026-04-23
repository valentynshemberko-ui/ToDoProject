
#include <gtest/gtest.h>
#include "../databaseManager/databaseManager.h"
#include "../databaseManager/accountRepository.h"
#include "../databaseManager/TaskRepository.h"
#include "../tasks/TaskManager.h"
#include "../tasks/task.h"

TEST(Integration, RegisterLoginAddCompleteTask) {
    DatabaseManager db(":memory:");
    ASSERT_TRUE(db.open());
    AccountRepository accRepo(db); accRepo.initTable();
    TaskRepository taskRepo(db); taskRepo.initTable();
    TaskManager mgr(taskRepo);
    mgr.setCurrentUser("denys");

    ASSERT_TRUE(accRepo.addAccount("denys", std::hash<std::string>{}("pwd")));
    ASSERT_TRUE(accRepo.accountExists("denys"));

    Task t("DoWork","desc", QDateTime::currentDateTime(), "High", false);
    ASSERT_TRUE(mgr.addTask(nullptr, t));
    auto list = mgr.loadTasks();
    ASSERT_EQ(list.size(), 1u);
    EXPECT_EQ(list[0].getTitle(), "DoWork");

    ASSERT_TRUE(mgr.markCompleted(nullptr, "DoWork"));
    auto after = mgr.loadTasks();
    ASSERT_EQ(after.size(), 1u);
    EXPECT_TRUE(after[0].isCompleted());
}
