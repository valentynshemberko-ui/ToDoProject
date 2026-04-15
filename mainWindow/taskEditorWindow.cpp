#include "taskeditorwindow.h"
#include "ui_taskeditorwindow.h"
#include "../settings/appSettings.h"
#include <QDate>
#include <QMenu>
#include <QWidgetAction>
#include <QTime>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>

TaskEditorWindow::TaskEditorWindow(QWidget *parent)
    : FramelessDialog(parent),
      ui(new Ui::TaskEditorWindow)
{
    ui->setupUi(this);
    setWindowTitle("New Task");
    resize(400, 300);
    if (AppSettings::theme() == AppSettings::Theme::Light)
        ui->btnClose->setIcon(QIcon(":/resources/icons/icons-for-window/close-black.png"));
    else
        ui->btnClose->setIcon(QIcon(":/resources/icons/icons-for-window/close-white.png"));


    if (AppSettings::theme() == AppSettings::Theme::Light) ui->btnChangeTime->setIcon(QIcon(":/resources/icons/time-black.png"));
    else ui->btnChangeTime->setIcon(QIcon(":/resources/icons/time-white.png"));
    ui->btnChangeTime->setIconSize(QSize(20, 20));

    connect(ui->btnChangeTime, &QPushButton::clicked, this,
        [=]()
{
    QMenu *menu = new QMenu(ui->deadlineEdit);
    menu->setWindowFlags(menu->windowFlags() | Qt::FramelessWindowHint);
    menu->setAttribute(Qt::WA_TranslucentBackground);

    auto *shadow = new QGraphicsDropShadowEffect(menu);
    shadow->setBlurRadius(20);
    shadow->setOffset(0, 3);
    shadow->setColor(QColor(0, 0, 0, 100));
menu->setGraphicsEffect(shadow);
    QWidget *popup = new QWidget(menu);
    popup->setMinimumWidth(220);

    QVBoxLayout *vbox = new QVBoxLayout(popup);
    vbox->setContentsMargins(10, 10, 10, 10);
    vbox->setSpacing(10);

    QLabel *lbl = new QLabel("Select time", popup);

    QSlider *hourSlider = new QSlider(Qt::Horizontal, popup);
    hourSlider->setRange(0, 23);
    hourSlider->setValue(ui->deadlineEdit->time().hour());
    QLabel *hourLabel = new QLabel(QString("Hour: %1").arg(QString::number(hourSlider->value())), popup);

    QSlider *minSlider = new QSlider(Qt::Horizontal, popup);
    minSlider->setRange(0, 59);
    minSlider->setValue(ui->deadlineEdit->time().minute());
    QLabel *minLabel = new QLabel(QString("Minute: %1").arg(QString::number(minSlider->value())), popup);

    connect(hourSlider, &QSlider::valueChanged, hourLabel, [=](int v) {
        QString numberString = QString::number(v).rightJustified(2, QLatin1Char('0'));
        hourLabel->setText(QString("Hour: %1").arg(numberString));
    });
    connect(minSlider, &QSlider::valueChanged, minLabel, [=](int v) {
        QString numberString = QString::number(v).rightJustified(2, QLatin1Char('0'));
        minLabel->setText(QString("Minute: %1").arg(numberString));
    });

    QPushButton *okBtn = new QPushButton("OK", popup);
    QPushButton *cancelBtn = new QPushButton("Cancel", popup);
    okBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setCursor(Qt::PointingHandCursor);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);

    vbox->addWidget(lbl);
    vbox->addWidget(hourLabel);
    vbox->addWidget(hourSlider);
    vbox->addWidget(minLabel);
    vbox->addWidget(minSlider);
    vbox->addLayout(btnLayout);

    QWidgetAction *act = new QWidgetAction(menu);
    act->setDefaultWidget(popup);
    menu->addAction(act);

    connect(okBtn, &QPushButton::clicked, menu, [=]() {
        QTime newTime(hourSlider->value(), minSlider->value());
        ui->deadlineEdit->setTime(newTime);
        menu->close();
    });
    connect(cancelBtn, &QPushButton::clicked, menu, [=]() {
        menu->close();
    });

    QPoint pos = ui->deadlineEdit->mapToGlobal(QPoint(0, ui->deadlineEdit->height() + 2));
    menu->exec(pos);
});

    ui->deadlineEdit->setDateTime(QDateTime::currentDateTime());
    ui->priorityBox->addItems({"Low", "Medium", "High"});

    connect(ui->saveButton, &QPushButton::clicked, this, &TaskEditorWindow::saveTask);
    connect(ui->cancelButton, &QPushButton::clicked, this, &TaskEditorWindow::reject);
    setupTitleBar();
}

TaskEditorWindow::~TaskEditorWindow() {
    delete ui;
}

void TaskEditorWindow::setupTitleBar() {
    ui->titleBar->setMinimumHeight(36);
    ui->titleBar->setMaximumHeight(36);
    connect(ui->btnClose,&QPushButton::clicked, this, &reject);
}

void TaskEditorWindow::saveTask() {
    QString title = ui->titleEdit->text().trimmed();
    if (title.isEmpty()) {
        FramelessMessageBox::warning(this, "Warning", "Task title cannot be empty!");
        return;
    }
    accept();
}

Task TaskEditorWindow::getTask() const {
    return Task(
        ui->titleEdit->text(),
        ui->descriptionEdit->toPlainText(),
        ui->deadlineEdit->dateTime(),
        ui->priorityBox->currentText(),
        false
    );
}

void TaskEditorWindow::setTask(const Task& task) {
    ui->titleEdit->setText(task.getTitle());
    ui->descriptionEdit->setText(task.getDescription());
    ui->deadlineEdit->setDateTime(task.getDeadline());
    int index = ui->priorityBox->findText(task.getPriority());
    if (index >= 0) ui->priorityBox->setCurrentIndex(index);
}
