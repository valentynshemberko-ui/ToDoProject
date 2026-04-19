#include "settingswindow.h"
#include "ui_settingswindow.h"
#include "appsettings.h"
#include <QFileDialog>
#include <QIcon>

SettingsWindow::SettingsWindow(QWidget *parent)
    : FramelessDialog(parent), ui(new Ui::SettingsWindow) {
    ui->setupUi(this);

    audioOut = new QAudioOutput(this);
    audioOut->setVolume(20);
    player = new QMediaPlayer(this);
    player->setAudioOutput(audioOut);

    if (AppSettings::theme() == AppSettings::Theme::Light)
        ui->btnClose->setIcon(QIcon(":/resources/icons/icons-for-window/close-black.png"));
    else
        ui->btnClose->setIcon(QIcon(":/resources/icons/icons-for-window/close-white.png"));

    connect(ui->testSoundButton, &QPushButton::clicked, this, &SettingsWindow::onTestSound);

    ui->defaultDeadlineBox->addItems({"None", "Today", "Tomorrow", "In 3 days", "In a week"});
    ui->deletePeriodBox->addItems({"Immediately", "After 1 day", "After 1 week", "After deadline passes"});
    ui->themeBox->addItems({"Light", "Dark"});

    loadUiFromSettings();
    applyVisibility();
    setupTitleBar();

    connect(ui->saveButton, &QPushButton::clicked, this, &SettingsWindow::onSave);
    connect(ui->cancelButton, &QPushButton::clicked, this, &SettingsWindow::onCancel);
    connect(ui->selectSoundButton, &QPushButton::clicked, this, &SettingsWindow::onSelectSound);
    connect(ui->autoDeleteCheck, &QCheckBox::stateChanged, this, &SettingsWindow::onAutoDeleteChanged);
    connect(ui->reminderCheck, &QCheckBox::stateChanged, this, &SettingsWindow::onReminderChanged);
    connect(ui->testSoundButton, &QPushButton::clicked, this, &SettingsWindow::onTestSound);
}

SettingsWindow::~SettingsWindow() {
    delete ui;
}

void SettingsWindow::loadUiFromSettings() {
    const QString dd = AppSettings::s().value("defaultDeadline", "Tomorrow").toString();
    ui->defaultDeadlineBox->setCurrentText(dd);

    ui->autoDeleteCheck->setChecked(AppSettings::autoDelete());
    ui->deletePeriodBox->setCurrentText(AppSettings::s().value("deletePeriod", "After deadline passes").toString());

    ui->themeBox->setCurrentText(AppSettings::theme() == AppSettings::Theme::Dark ? "Dark" : "Light");

    ui->reminderCheck->setChecked(AppSettings::reminderEnabled());
    ui->reminderTimeBox->setValue(AppSettings::reminderMinutes());
    const QString snd = AppSettings::reminderSound();
    ui->labelSoundPath->setText(snd.isEmpty() ? "(none selected)" : snd);
}

void SettingsWindow::applyVisibility() {
    const bool showDel = ui->autoDeleteCheck->isChecked();
    ui->labelDeletePeriod->setVisible(showDel);
    ui->deletePeriodBox->setVisible(showDel);

    const bool showRem = ui->reminderCheck->isChecked();
    ui->labelReminderTime->setVisible(showRem);
    ui->reminderTimeBox->setVisible(showRem);
    ui->labelReminderSound->setVisible(showRem);
    ui->selectSoundButton->setVisible(showRem);
    ui->labelSoundPath->setVisible(showRem);
    ui->labelSoundPath_2->setVisible(showRem);
    ui->testLabel->setVisible(showRem);
    ui->testSoundButton->setVisible(showRem);
    ui->testSoundButton->setVisible(showRem);
}

void SettingsWindow::onAutoDeleteChanged(int) { applyVisibility(); }
void SettingsWindow::onReminderChanged(int)   { applyVisibility(); }
void SettingsWindow::onTestSound() {
    const QString path = ui->labelSoundPath->text();
    if (path.isEmpty() || path == "(none selected)" || !QFileInfo::exists(path)) {
        FramelessMessageBox::warning(this, "No sound", "Please select a valid audio file first.");

        return;
    }

    player->stop();
    player->setSource(QUrl::fromLocalFile(path));
    player->play();
}

void SettingsWindow::onSelectSound() {
    QString f = QFileDialog::getOpenFileName(this, "Select Reminder Sound", "", "Audio Files (*.mp3 *.wav *.ogg *.flac)");
    if (!f.isEmpty()) {
        ui->labelSoundPath->setText(f);
        ui->testSoundButton->setEnabled(true);
    }
}

void SettingsWindow::onSave() {
    AppSettings::s().setValue("defaultDeadline", ui->defaultDeadlineBox->currentText());

    AppSettings::s().setValue("autoDelete", ui->autoDeleteCheck->isChecked());
    AppSettings::s().setValue("deletePeriod", ui->deletePeriodBox->currentText());

    AppSettings::s().setValue("theme", ui->themeBox->currentText());

    AppSettings::s().setValue("reminderEnabled", ui->reminderCheck->isChecked());
    AppSettings::s().setValue("reminderTime", ui->reminderTimeBox->value());
    const QString sound = ui->labelSoundPath->text();
    AppSettings::s().setValue("reminderSound", sound == "(none selected)" ? "" : sound);

    accept();
}

void SettingsWindow::setupTitleBar() {
    ui->titleBar->setMinimumHeight(36);
    ui->titleBar->setMaximumHeight(36);
    connect(ui->btnClose,    &QPushButton::clicked, this, &onCancel);

}

void SettingsWindow::onCancel() { reject(); }
