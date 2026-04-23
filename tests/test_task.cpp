
#include <gtest/gtest.h>
#include <QString>
#include <QDateTime>
#include "../tasks/task.h"

TEST(TaskModel, DefaultConstructor) {
    Task t;
    EXPECT_TRUE(t.getTitle().isEmpty());
    EXPECT_TRUE(t.getDescription().isEmpty());
    EXPECT_FALSE(t.isCompleted());
    EXPECT_EQ(t.getPriority(), "Medium");
    auto dt = t.getDeadline();
    EXPECT_TRUE(dt.isValid());
}

TEST(TaskModel, CustomConstructorAndGetters) {
    QDateTime dl = QDateTime::currentDateTime().addDays(2);
    Task t("Title", "Desc", dl, "High", true);
    EXPECT_EQ(t.getTitle(), "Title");
    EXPECT_EQ(t.getDescription(), "Desc");
    EXPECT_EQ(t.getDeadline(), dl);
    EXPECT_EQ(t.getPriority(), "High");
    EXPECT_TRUE(t.isCompleted());
}

TEST(TaskModel, Setters) {
    Task t;
    QDateTime dl = QDateTime::fromString("2025-10-30T14:30:00", Qt::ISODate);
    t.setTitle("X");
    t.setDescription("Y");
    t.setDeadline(dl);
    t.setPriority("Low");
    t.setCompleted(true);
    EXPECT_EQ(t.getTitle(), "X");
    EXPECT_EQ(t.getDescription(), "Y");
    EXPECT_EQ(t.getDeadline(), dl);
    EXPECT_EQ(t.getPriority(), "Low");
    EXPECT_TRUE(t.isCompleted());
}

TEST(TaskModel, ToDisplayString) {
    QDateTime dl = QDateTime::fromString("2025-10-30T14:30:00", Qt::ISODate);
    Task t("Do", "Something", dl, "Mid", false);
    QString s = t.toDisplayString();
    EXPECT_TRUE(s.contains("Do"));
    EXPECT_TRUE(s.contains("Mid"));
    EXPECT_TRUE(s.contains("30.10.2025"));
}
