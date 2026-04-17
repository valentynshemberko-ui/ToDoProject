#ifndef TASKS_H
#define TASKS_H

#include <QString>
#include <QDate>

class Task {
private:
    QString title;
    QString description;
    QDateTime deadline;
    QString priority;
    bool completed;

public:
    Task();
    Task(const QString &title, const QString &description,
         const QDateTime &deadline, const QString &priority, bool completed = false);

    QString getTitle() const;
    QString getDescription() const;
    const QDateTime& getDeadline() const;
    QString getPriority() const;
    bool isCompleted() const;

    void setTitle(const QString &value);
    void setDescription(const QString &value);
    void setDeadline(const QDateTime& d);
    void setPriority(const QString &value);
    void setCompleted(bool value);

    QString toDisplayString() const;
};


#endif
