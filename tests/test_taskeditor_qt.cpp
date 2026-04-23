#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QPushButton>
#include <QMenu>
#include <QSlider>
#include <QTimer>
#include <QApplication>
#include "../mainWindow/taskEditorWindow.h"
#include "../tasks/task.h"

TEST(TaskEditorWindowQt, EmitsAcceptedWhenSaving) {
    TaskEditorWindow w;
    QSignalSpy spy(&w, &QDialog::accepted);

    auto title  = w.findChild<QLineEdit*>("titleEdit");
    auto desc   = w.findChild<QTextEdit*>("descriptionEdit");
    auto dt     = w.findChild<QDateTimeEdit*>("deadlineEdit");
    auto prio   = w.findChild<QComboBox*>("priorityBox");
    auto btn    = w.findChild<QPushButton*>("saveButton");

    ASSERT_TRUE(title);
    ASSERT_TRUE(desc);
    ASSERT_TRUE(dt);
    ASSERT_TRUE(prio);
    ASSERT_TRUE(btn);

    title->setText("Test Task");
    desc->setPlainText("Desc");
    dt->setDateTime(QDateTime::fromString("2025-10-30T12:00:00", Qt::ISODate));
    int idx = prio->findText("High");
    if (idx < 0) idx = 0;
    prio->setCurrentIndex(idx);

    btn->click();
    QCoreApplication::processEvents();

    EXPECT_GE(spy.count(), 1) << "Dialog should emit accepted() after valid save";
}

TEST(TaskEditorWindowQt, SetAndGetTask) {
    TaskEditorWindow w;

    QDateTime testDateTime = QDateTime::fromString("2025-12-31T23:59:00", Qt::ISODate);
    Task originalTask("Test Title", "Test Description", testDateTime, "High", false);

    w.setTask(originalTask);

    auto titleEdit = w.findChild<QLineEdit*>("titleEdit");
    ASSERT_TRUE(titleEdit);
    EXPECT_EQ(titleEdit->text(), "Test Title");

    Task retrievedTask = w.getTask();

    EXPECT_EQ(retrievedTask.getTitle(), "Test Title");
    EXPECT_EQ(retrievedTask.getDescription(), "Test Description");
    EXPECT_EQ(retrievedTask.getDeadline(), testDateTime);
    EXPECT_EQ(retrievedTask.getPriority(), "High");
    EXPECT_FALSE(retrievedTask.isCompleted());
}

TEST(TaskEditorWindowQt, ChangeTimeMenuAppliesTimeOnOk) {
    TaskEditorWindow w;
    w.show();

    auto btnChangeTime = w.findChild<QPushButton*>("btnChangeTime");
    auto deadlineEdit = w.findChild<QDateTimeEdit*>("deadlineEdit");

    ASSERT_TRUE(btnChangeTime);
    ASSERT_TRUE(deadlineEdit);

    QTime initialTime(10, 0);
    deadlineEdit->setTime(initialTime);

    QTimer::singleShot(100, [&]() {
        QMenu* activeMenu = nullptr;
        for (QWidget* widget : QApplication::topLevelWidgets()) {
            activeMenu = qobject_cast<QMenu*>(widget);
            if (activeMenu && activeMenu->isVisible()) {
                break;
            }
        }
        ASSERT_TRUE(activeMenu != nullptr) << "Menu was not found or not visible";

        QList<QSlider*> sliders = activeMenu->findChildren<QSlider*>();
        ASSERT_GE(sliders.size(), 2) << "Sliders not found in the menu";

        QSlider* hourSlider = sliders[0];
        QSlider* minSlider = sliders[1];

        hourSlider->setValue(15);
        minSlider->setValue(30);

        QList<QPushButton*> buttons = activeMenu->findChildren<QPushButton*>();
        QPushButton* okBtn = nullptr;
        for (QPushButton* btn : buttons) {
            if (btn->text() == "OK") {
                okBtn = btn;
                break;
            }
        }
        ASSERT_TRUE(okBtn != nullptr) << "OK button not found";

        okBtn->click();
    });

    btnChangeTime->click();
    QCoreApplication::processEvents();

    EXPECT_EQ(deadlineEdit->time().hour(), 15);
    EXPECT_EQ(deadlineEdit->time().minute(), 30);
}

TEST(TaskEditorWindowQt, ChangeTimeMenuDoesNotChangeTimeOnCancel) {
    TaskEditorWindow w;
    w.show();

    auto btnChangeTime = w.findChild<QPushButton*>("btnChangeTime");
    auto deadlineEdit = w.findChild<QDateTimeEdit*>("deadlineEdit");

    ASSERT_TRUE(btnChangeTime);
    ASSERT_TRUE(deadlineEdit);

    QTime initialTime(10, 0);
    deadlineEdit->setTime(initialTime);

    QTimer::singleShot(100, [&]() {
        QMenu* activeMenu = nullptr;
        for (QWidget* widget : QApplication::topLevelWidgets()) {
            activeMenu = qobject_cast<QMenu*>(widget);
            if (activeMenu && activeMenu->isVisible()) {
                break;
            }
        }
        ASSERT_TRUE(activeMenu != nullptr);

        QList<QSlider*> sliders = activeMenu->findChildren<QSlider*>();
        ASSERT_GE(sliders.size(), 2);

        QSlider* hourSlider = sliders[0];
        hourSlider->setValue(20);

        QList<QPushButton*> buttons = activeMenu->findChildren<QPushButton*>();
        QPushButton* cancelBtn = nullptr;
        for (QPushButton* btn : buttons) {
            if (btn->text() == "Cancel") {
                cancelBtn = btn;
                break;
            }
        }
        ASSERT_TRUE(cancelBtn != nullptr) << "Cancel button not found";

        cancelBtn->click();
    });

    btnChangeTime->click();
    QCoreApplication::processEvents();

    EXPECT_EQ(deadlineEdit->time().hour(), 10);
    EXPECT_EQ(deadlineEdit->time().minute(), 0);
}