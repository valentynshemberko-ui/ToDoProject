#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QSettings>
#include <QDateTime>
#include <QString>

class AppSettings {
public:
    enum class DefaultDeadline { None, Today, Tomorrow, In3Days, InWeek };
    enum class AutoDeletePeriod { Immediately, After1Day, After1Week, AfterDeadline };
    enum class Theme { Light, Dark };
    enum class UserStatus { Active, Busy, Away };

    static QSettings& s();

    static DefaultDeadline defaultDeadline();
    static void setDefaultDeadline(DefaultDeadline d);

    static bool autoDelete();
    static void setAutoDelete(bool on);

    static AutoDeletePeriod deletePeriod();
    static void setDeletePeriod(AutoDeletePeriod p);

    static Theme theme();
    static void setTheme(Theme t);

    static bool reminderEnabled();
    static void setReminderEnabled(bool on);

    static int reminderMinutes();
    static void setReminderMinutes(int m);

    static QString reminderSound();
    static void setReminderSound(const QString& path);

    static QDateTime computeQuickAddDeadline(QDateTime now = QDateTime::currentDateTime());

    static UserStatus userStatus();
    static void setUserStatus(UserStatus status);
    static QString userStatusEmoji();
    static QString userStatusName();
};

#endif // APPSETTINGS_H
