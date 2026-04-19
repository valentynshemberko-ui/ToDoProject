#include "appSettings.h"

QSettings& AppSettings::s() {
    static QSettings settings("ToDoSoft", "ToDoManager");
    return settings;
}

AppSettings::DefaultDeadline AppSettings::defaultDeadline() {
    const QString v = s().value("defaultDeadline", "Tomorrow").toString();
    if (v == "Today") return DefaultDeadline::Today;
    if (v == "In 3 days") return DefaultDeadline::In3Days;
    if (v == "In a week") return DefaultDeadline::InWeek;
    if (v == "None") return DefaultDeadline::None;
    return DefaultDeadline::Tomorrow;
}

void AppSettings::setDefaultDeadline(DefaultDeadline d) {
    QString v = "Tomorrow";
    if (d == DefaultDeadline::None) v = "None";
    else if (d == DefaultDeadline::Today) v = "Today";
    else if (d == DefaultDeadline::In3Days) v = "In 3 days";
    else if (d == DefaultDeadline::InWeek) v = "In a week";
    s().setValue("defaultDeadline", v);
}

bool AppSettings::autoDelete() { return s().value("autoDelete", false).toBool(); }
void AppSettings::setAutoDelete(bool on) { s().setValue("autoDelete", on); }

AppSettings::AutoDeletePeriod AppSettings::deletePeriod() {
    const QString v = s().value("deletePeriod", "After deadline passes").toString();
    if (v == "Immediately") return AutoDeletePeriod::Immediately;
    if (v == "After 1 day") return AutoDeletePeriod::After1Day;
    if (v == "After 1 week") return AutoDeletePeriod::After1Week;
    return AutoDeletePeriod::AfterDeadline;
}

void AppSettings::setDeletePeriod(AutoDeletePeriod p) {
    QString v = "After deadline passes";
    if (p == AutoDeletePeriod::Immediately) v = "Immediately";
    else if (p == AutoDeletePeriod::After1Day) v = "After 1 day";
    else if (p == AutoDeletePeriod::After1Week) v = "After 1 week";
    s().setValue("deletePeriod", v);
}

AppSettings::Theme AppSettings::theme() {
    const QString v = s().value("theme", "Light").toString();
    return (v == "Dark") ? Theme::Dark : Theme::Light;
}
void AppSettings::setTheme(Theme t) {
    s().setValue("theme", t == Theme::Dark ? "Dark" : "Light");
}

bool AppSettings::reminderEnabled() { return s().value("reminderEnabled", false).toBool(); }
void AppSettings::setReminderEnabled(bool on) { s().setValue("reminderEnabled", on); }

int AppSettings::reminderMinutes() { return s().value("reminderTime", 30).toInt(); }
void AppSettings::setReminderMinutes(int m) { s().setValue("reminderTime", m); }

QString AppSettings::reminderSound() { return s().value("reminderSound", "").toString(); }
void AppSettings::setReminderSound(const QString& path) { s().setValue("reminderSound", path); }

QDateTime AppSettings::computeQuickAddDeadline(QDateTime now) {
    QDate today = now.date();
    QDateTime base(today, QTime(23, 59));

    switch (defaultDeadline()) {
        case DefaultDeadline::None:
            return QDateTime();
        case DefaultDeadline::Today:
            return base;
        case DefaultDeadline::Tomorrow:
            return base.addDays(1);
        case DefaultDeadline::In3Days:
            return base.addDays(3);
        case DefaultDeadline::InWeek:
            return base.addDays(7);
    }
    return base;
}

AppSettings::UserStatus AppSettings::userStatus() {
    const QString v = s().value("userStatus", "Active").toString();
    if (v == "Busy") return UserStatus::Busy;
    if (v == "Away") return UserStatus::Away;
    return UserStatus::Active;
}

void AppSettings::setUserStatus(UserStatus status) {
    QString v = "Active";
    if (status == UserStatus::Busy) v = "Busy";
    else if (status == UserStatus::Away) v = "Away";
    s().setValue("userStatus", v);
}

QString AppSettings::userStatusEmoji() {
    switch (userStatus()) {
        case UserStatus::Busy: return "🔴";
        case UserStatus::Away: return "🟡";
        case UserStatus::Active:
        default: return "🟢";
    }
}

QString AppSettings::userStatusName() {
    switch (userStatus()) {
        case UserStatus::Busy: return "Busy";
        case UserStatus::Away: return "Away";
        case UserStatus::Active:
        default: return "Active";
    }
}
