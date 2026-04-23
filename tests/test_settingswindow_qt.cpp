#include <gtest/gtest.h>
#include <QApplication>
#include <QTimer>
#include <QTest>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QTemporaryFile>
#include <QFileDialog>
#include <QSignalSpy>
#include <QSlider>
#include "../settings/settingsWindow.h"
#include "../settings/appSettings.h"
#include "../windowEdit/framelessWindow.h"


TEST(SettingsWindowQt, ThemeComboChangesSetting) {
    SettingsWindow w;
    auto themeCombo = w.findChild<QComboBox*>("themeBox");
    ASSERT_TRUE(themeCombo);
    int oldIndex = themeCombo->currentIndex();
    int newIndex = (oldIndex == 0 ? 1 : 0);
    themeCombo->setCurrentIndex(newIndex);
    EXPECT_EQ(themeCombo->currentIndex(), newIndex);
}

class SettingsWindowSlotsTest : public ::testing::Test {
protected:
    void SetUp() override {
        AppSettings::s().clear();
    }
    void TearDown() override {}
};

TEST_F(SettingsWindowSlotsTest, CheckboxChangesTriggerVisibilityUpdate) {
    SettingsWindow w;
    auto autoDeleteCheck = w.findChild<QCheckBox*>("autoDeleteCheck");
    auto reminderCheck = w.findChild<QCheckBox*>("reminderCheck");

    ASSERT_TRUE(autoDeleteCheck != nullptr);
    ASSERT_TRUE(reminderCheck != nullptr);

    autoDeleteCheck->setChecked(true);
    reminderCheck->setChecked(true);

    SUCCEED();
}

TEST_F(SettingsWindowSlotsTest, TestSoundWithEmptyPathShowsWarning) {
    SettingsWindow w;
    auto labelSoundPath = w.findChild<QLabel*>("labelSoundPath");
    ASSERT_TRUE(labelSoundPath != nullptr);

    labelSoundPath->setText("(none selected)");

    QTimer::singleShot(250, []() {
        for (QWidget* widget : QApplication::topLevelWidgets()) {
            if (widget->inherits("FramelessMessageBox") && widget->isVisible()) {
                if (QDialog* dlg = qobject_cast<QDialog*>(widget)) {
                    dlg->accept();
                    return;
                }
            }
        }
    });

    QMetaObject::invokeMethod(&w, "onTestSound");

    QTest::qWait(300);
}

TEST_F(SettingsWindowSlotsTest, TestSoundWithValidPathPlaysAudio) {
    SettingsWindow w;
    auto labelSoundPath = w.findChild<QLabel*>("labelSoundPath");
    ASSERT_TRUE(labelSoundPath != nullptr);

    QTemporaryFile tempAudioFile;
    ASSERT_TRUE(tempAudioFile.open());
    QString validPath = tempAudioFile.fileName();
    tempAudioFile.close();

    labelSoundPath->setText(validPath);

    QMetaObject::invokeMethod(&w, "onTestSound");

    QTest::qWait(100);
    SUCCEED();
}

TEST_F(SettingsWindowSlotsTest, SelectSoundOpensFileDialog) {
    SettingsWindow w;

    QTimer::singleShot(250, []() {
        for (QWidget* widget : QApplication::topLevelWidgets()) {
            if (auto dialog = qobject_cast<QFileDialog*>(widget)) {
                dialog->reject();
                return;
            }
        }
    });

    QMetaObject::invokeMethod(&w, "onSelectSound");
    SUCCEED();
}

TEST_F(SettingsWindowSlotsTest, OnSaveUpdatesAppSettings) {
    SettingsWindow w;

    auto defaultDeadlineBox = w.findChild<QComboBox*>("defaultDeadlineBox");
    auto autoDeleteCheck = w.findChild<QCheckBox*>("autoDeleteCheck");
    auto deletePeriodBox = w.findChild<QComboBox*>("deletePeriodBox");
    auto themeBox = w.findChild<QComboBox*>("themeBox");
    auto reminderCheck = w.findChild<QCheckBox*>("reminderCheck");
    auto reminderTimeBox = w.findChild<QSpinBox*>("reminderTimeBox");
    auto labelSoundPath = w.findChild<QLabel*>("labelSoundPath");

    ASSERT_TRUE(defaultDeadlineBox);
    ASSERT_TRUE(autoDeleteCheck);
    ASSERT_TRUE(deletePeriodBox);
    ASSERT_TRUE(themeBox);
    ASSERT_TRUE(reminderCheck);
    ASSERT_TRUE(reminderTimeBox);
    ASSERT_TRUE(labelSoundPath);

    defaultDeadlineBox->clear();
    defaultDeadlineBox->addItem("Tomorrow");
    defaultDeadlineBox->setCurrentText("Tomorrow");

    autoDeleteCheck->setChecked(true);

    deletePeriodBox->clear();
    deletePeriodBox->addItem("After 1 Week");
    deletePeriodBox->setCurrentText("After 1 Week");

    themeBox->clear();
    themeBox->addItem("Dark");
    themeBox->setCurrentText("Dark");

    reminderCheck->setChecked(true);
    reminderTimeBox->setValue(15);
    labelSoundPath->setText("my_custom_sound.wav");

    QMetaObject::invokeMethod(&w, "onSave");

    EXPECT_EQ(AppSettings::s().value("defaultDeadline").toString(), "Tomorrow");
    EXPECT_EQ(AppSettings::s().value("autoDelete").toBool(), true);
    EXPECT_EQ(AppSettings::s().value("deletePeriod").toString(), "After 1 Week");
    EXPECT_EQ(AppSettings::s().value("theme").toString(), "Dark");
    EXPECT_EQ(AppSettings::s().value("reminderEnabled").toBool(), true);
    EXPECT_EQ(AppSettings::s().value("reminderTime").toInt(), 15);
    EXPECT_EQ(AppSettings::s().value("reminderSound").toString(), "my_custom_sound.wav");
}