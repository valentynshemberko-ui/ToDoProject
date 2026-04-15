#include "taskitemwidget.h"
#include "ui_taskitemwidget.h"
#include "../settings/appSettings.h"
#include <string>
#include <QHBoxLayout>
#include <QDate>
#include <QEnterEvent>
#include <QMouseEvent>
#include <QFont>
#include <QPixmap>
#include <QStyle>

using namespace std;

TaskItemWidget::TaskItemWidget(const Task& t, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::TaskItemWidget),
      task(t),
      btnEffect(new QGraphicsOpacityEffect(this)),
      fadeAnim(new QPropertyAnimation(btnEffect, "opacity", this))
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);

    QWidget *btnContainer = new QWidget(this);
    btnContainer->setObjectName("btnContainerStyle");
    QHBoxLayout *btnLayout = new QHBoxLayout(btnContainer);
    btnLayout->addWidget(ui->editButton);
    btnLayout->addWidget(ui->doneButton);
    btnLayout->addWidget(ui->detailsButton);
    btnLayout->addWidget(ui->deleteButton);
    btnContainer->setGraphicsEffect(btnEffect);
    ui->mainLayout->addWidget(btnContainer);

    btnEffect->setOpacity(0.0);

    connect(ui->editButton,    &QPushButton::clicked, [this]() { emit requestEdit(task); });
    connect(ui->doneButton,    &QPushButton::clicked, [this]() { emit requestDone(task); });
    connect(ui->detailsButton, &QPushButton::clicked, [this]() { emit requestDetails(task); });
    connect(ui->deleteButton,  &QPushButton::clicked, [this]() { emit requestDelete(task); });

    updateDisplay();
    setStyleSheet(qApp->styleSheet());
}

TaskItemWidget::~TaskItemWidget() {
    delete ui;
}

Task TaskItemWidget::getTask() const {
    return task;
}

void TaskItemWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton)
        emit requestDetails(task);
    QWidget::mouseDoubleClickEvent(event);
}

void TaskItemWidget::updateDisplay() {
    ui->titleLabel->setText(task.getTitle());
    ui->titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QDateTime deadline = task.getDeadline();

    if (deadline.isValid()) {
        ui->labelDeadline->setText(deadline.toString("dd MMM yyyy  hh:mm"));
    } else {
        ui->labelDeadline->setText("No deadline");
    }

    if (task.isCompleted()) {
        applyDoneState();
    } else if (deadline.isValid() && deadline < QDateTime::currentDateTime()) {
        applyOverdueState();
    } else {
        applyNormalState();
    }

    style()->unpolish(this);
    style()->polish(this);
}

void TaskItemWidget::applyDoneState() {
    setProperty("state", "done");

    QString iconPath = (AppSettings::theme() == AppSettings::Theme::Dark)
                       ? ":/resources/icons/check-white.png"
                       : ":/resources/icons/check-black.png";

    ui->labelCheck->setPixmap(QPixmap(iconPath).scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->labelCheck->setVisible(true);
    ui->labelPriority->setVisible(false);
    ui->doneButton->setVisible(false);
}

void TaskItemWidget::applyOverdueState() {
    setProperty("state", "overdue");
    ui->labelCheck->setVisible(false);

    QString iconPath = (AppSettings::theme() == AppSettings::Theme::Dark)
                       ? ":/resources/icons/overdue-white.png"
                       : ":/resources/icons/overdue-black.png";

    ui->labelPriority->setPixmap(QPixmap(iconPath).scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->labelPriority->setVisible(true);

    ui->doneButton->setVisible(true);
    ui->doneButton->setEnabled(true);
    ui->doneButton->setToolTip("Task is overdue! Mark as done.");
}

void TaskItemWidget::applyNormalState() {
    setProperty("state", "normal");
    ui->labelCheck->setVisible(false);

    QString priority = task.getPriority();
    QString iconPath;

    if (priority == "Low")         iconPath = ":/resources/icons/priority_low.png";
    else if (priority == "Medium") iconPath = ":/resources/icons/priority_medium.png";
    else if (priority == "High")   iconPath = ":/resources/icons/priority_high.png";

    if (!iconPath.isEmpty()) {
        ui->labelPriority->setPixmap(QPixmap(iconPath).scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->labelPriority->setVisible(true);
    } else {
        ui->labelPriority->setVisible(false);
    }

    ui->doneButton->setVisible(true);
    ui->doneButton->setEnabled(true);
    ui->doneButton->setToolTip("Mark task as done");
}
void TaskItemWidget::setButtonsVisible(bool visible) {
    fadeAnim->stop();
    fadeAnim->setDuration(250);
    fadeAnim->setStartValue(btnEffect->opacity());
    fadeAnim->setEndValue(visible ? 1.0 : 0.0);
    fadeAnim->start();
}

void TaskItemWidget::enterEvent(QEnterEvent *event) {
    setButtonsVisible(true);
    QWidget::enterEvent(event);
}

void TaskItemWidget::leaveEvent(QEvent *event) {
    setButtonsVisible(false);
    QWidget::leaveEvent(event);
}
