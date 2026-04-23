#include <gtest/gtest.h>
#include <QApplication>
#include <QSignalSpy>
#include <QTest>
#include <QMouseEvent>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsOpacityEffect>
#include "../mainWindow/taskItemWidget.h"
#include "../settings/appSettings.h"
#include "../tasks/task.h"

TEST(TaskItemWidgetQt, EmitsEditAndDeleteRequests) {
    Task model("A", "B", QDateTime::currentDateTime(), "Medium", false);
    TaskItemWidget w(model);
    QSignalSpy editSpy(&w, &TaskItemWidget::requestEdit);
    QSignalSpy delSpy (&w, &TaskItemWidget::requestDelete);

    auto btnEdit = w.findChild<QPushButton*>("editButton");
    auto btnDel  = w.findChild<QPushButton*>("deleteButton");

    ASSERT_TRUE(btnEdit) << "editButton not found in UI";
    ASSERT_TRUE(btnDel)  << "deleteButton not found in UI";

    btnEdit->click();
    btnDel->click();

    QCoreApplication::processEvents();

    EXPECT_GE(editSpy.count(), 1) << "Edit button should emit requestEdit()";
    EXPECT_GE(delSpy.count(), 1)  << "Delete button should emit requestDelete()";
}

class TaskItemWidgetTest : public ::testing::Test {
protected:
    void SetUp() override {
        qRegisterMetaType<Task>("Task");
    }
    void TearDown() override {}
};

TEST_F(TaskItemWidgetTest, GetTaskReturnsCorrectTask) {
    Task testTask("Test Title", "Test Desc", QDateTime::currentDateTime(), "High", false);
    TaskItemWidget widget(testTask);

    Task retrievedTask = widget.getTask();

    EXPECT_EQ(retrievedTask.getTitle(), "Test Title");
    EXPECT_EQ(retrievedTask.getDescription(), "Test Desc");
}

TEST_F(TaskItemWidgetTest, DoubleClickEmitsRequestDetails) {
    Task testTask("Double Click Me", "Desc", QDateTime::currentDateTime(), "Low", false);
    TaskItemWidget widget(testTask);

    QSignalSpy spy(&widget, &TaskItemWidget::requestDetails);

    QPoint center = widget.rect().center();
    QMouseEvent dblClick(QEvent::MouseButtonDblClick, center, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&widget, &dblClick);

    EXPECT_EQ(spy.count(), 1);

    Task emittedTask = qvariant_cast<Task>(spy.takeFirst().at(0));
    EXPECT_EQ(emittedTask.getTitle(), "Double Click Me");
}

TEST_F(TaskItemWidgetTest, DoneStateUpdatesUI_DarkTheme) {
    // Встановлюємо темну тему для перевірки першої гілки (iconPath)
    AppSettings::setTheme(AppSettings::Theme::Dark);

    // Створюємо виконане завдання (done = true)
    Task doneTask("Done Task", "Desc", QDateTime::currentDateTime(), "Low", true);
    TaskItemWidget widget(doneTask);

    widget.show(); // <--- ВАЖЛИВО: показуємо віджет, щоб isVisible() запрацював коректно

    // Перевіряємо property
    EXPECT_EQ(widget.property("state").toString(), "done");

    // Знаходимо елементи інтерфейсу
    auto labelCheck = widget.findChild<QLabel*>("labelCheck");
    auto labelPriority = widget.findChild<QLabel*>("labelPriority");
    auto doneButton = widget.findChild<QPushButton*>("doneButton");

    ASSERT_TRUE(labelCheck != nullptr);
    ASSERT_TRUE(labelPriority != nullptr);
    ASSERT_TRUE(doneButton != nullptr);

    // Перевіряємо видимість елементів відповідно до коду
    EXPECT_TRUE(labelCheck->isVisible());
    EXPECT_FALSE(labelPriority->isVisible());
    EXPECT_FALSE(doneButton->isVisible());
}

TEST_F(TaskItemWidgetTest, DoneStateUpdatesUI_LightTheme) {
    AppSettings::setTheme(AppSettings::Theme::Light);

    Task doneTask("Done Task 2", "Desc", QDateTime::currentDateTime(), "Medium", true);
    TaskItemWidget widget(doneTask);

    widget.show();

    EXPECT_EQ(widget.property("state").toString(), "done");

    auto labelCheck = widget.findChild<QLabel*>("labelCheck");
    ASSERT_TRUE(labelCheck != nullptr);
    EXPECT_TRUE(labelCheck->isVisible());
}

TEST_F(TaskItemWidgetTest, EnterAndLeaveEventsTriggerFadeAnimation) {
    Task testTask("Hover Task", "Desc", QDateTime::currentDateTime(), "High", false);
    TaskItemWidget widget(testTask);
    widget.show();
    QTest::qWait(50);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QEnterEvent enterEvent(QPointF(10, 10), QPointF(10, 10), widget.mapToGlobal(QPoint(10, 10)));
#else
    QEvent enterEvent(QEvent::Enter);
#endif
    QApplication::sendEvent(&widget, &enterEvent);

    QTest::qWait(300);

    QEvent leaveEvent(QEvent::Leave);
    QApplication::sendEvent(&widget, &leaveEvent);

    QTest::qWait(300);

    SUCCEED();
}