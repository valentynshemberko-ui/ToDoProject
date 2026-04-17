#include "task.h"

Task::Task() : title(""), description(""), deadline(QDateTime::currentDateTime()),
               priority("Medium"), completed(false) {}

Task::Task(const QString &title, const QString &description,
           const QDateTime &deadline, const QString &priority, bool completed)
    : title(title), description(description),
      deadline(deadline), priority(priority), completed(completed) {}

QString Task::getTitle() const { return title; }
QString Task::getDescription() const { return description; }
const QDateTime& Task::getDeadline() const { return deadline; }
QString Task::getPriority() const { return priority; }
bool Task::isCompleted() const { return completed; }

void Task::setTitle(const QString &value) { title = value; }
void Task::setDescription(const QString &value) { description = value; }
void Task::setDeadline(const QDateTime& d) { deadline = d; }
void Task::setPriority(const QString &value) { priority = value; }
void Task::setCompleted(bool value) { completed = value; }

QString Task::toDisplayString() const {
    QString status = completed ? "[✓]" : "[ ]";
    return QString("%1 %2 (%3, %4)")
            .arg(status)
            .arg(title)
            .arg(deadline.toString("dd.MM.yyyy hh:mm"))
            .arg(priority);
}
