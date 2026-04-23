#include <gtest/gtest.h>
#include <QDateTime>
#include "../settings/appSettings.h"

TEST(AppSettings, ThemeSetGet) {
    auto t0 = AppSettings::theme();

    AppSettings::setTheme(AppSettings::Theme::Light);
    EXPECT_EQ(AppSettings::theme(), AppSettings::Theme::Light);

    AppSettings::setTheme(AppSettings::Theme::Dark);
    EXPECT_EQ(AppSettings::theme(), AppSettings::Theme::Dark);

    AppSettings::setTheme(t0);
}

TEST(AppSettings, ReminderSoundAndMinutes) {
    QString p0 = AppSettings::reminderSound();
    int m0 = AppSettings::reminderMinutes();

    AppSettings::setReminderSound("C:/tmp/ring.mp3");
    AppSettings::setReminderMinutes(77);

    EXPECT_EQ(AppSettings::reminderSound(), QString("C:/tmp/ring.mp3"));
    EXPECT_EQ(AppSettings::reminderMinutes(), 77);

    AppSettings::setReminderMinutes(0);
    EXPECT_EQ(AppSettings::reminderMinutes(), 0);
    AppSettings::setReminderMinutes(120);
    EXPECT_EQ(AppSettings::reminderMinutes(), 120);

    AppSettings::setReminderSound(p0);
    AppSettings::setReminderMinutes(m0);
}

TEST(AppSettings, AutoDeleteToggle) {
    bool prev = AppSettings::autoDelete();
    AppSettings::setAutoDelete(true);
    EXPECT_TRUE(AppSettings::autoDelete());
    AppSettings::setAutoDelete(false);
    EXPECT_FALSE(AppSettings::autoDelete());
    AppSettings::setAutoDelete(prev);
}

TEST(AppSettings, DeletePeriodSetGet) {
    using P = AppSettings::AutoDeletePeriod;
    AppSettings::setDeletePeriod(P::Immediately);
    EXPECT_EQ(AppSettings::deletePeriod(), P::Immediately);

    AppSettings::setDeletePeriod(P::After1Day);
    EXPECT_EQ(AppSettings::deletePeriod(), P::After1Day);

    AppSettings::setDeletePeriod(P::After1Week);
    EXPECT_EQ(AppSettings::deletePeriod(), P::After1Week);

    AppSettings::setDeletePeriod(P::AfterDeadline);
    EXPECT_EQ(AppSettings::deletePeriod(), P::AfterDeadline);
}

TEST(AppSettings, DefaultDeadlineSetGet) {
    using D = AppSettings::DefaultDeadline;

    AppSettings::setDefaultDeadline(D::None);
    EXPECT_EQ(AppSettings::defaultDeadline(), D::None);

    AppSettings::setDefaultDeadline(D::Today);
    EXPECT_EQ(AppSettings::defaultDeadline(), D::Today);

    AppSettings::setDefaultDeadline(D::Tomorrow);
    EXPECT_EQ(AppSettings::defaultDeadline(), D::Tomorrow);

    AppSettings::setDefaultDeadline(D::In3Days);
    EXPECT_EQ(AppSettings::defaultDeadline(), D::In3Days);

    AppSettings::setDefaultDeadline(D::InWeek);
    EXPECT_EQ(AppSettings::defaultDeadline(), D::InWeek);
}

TEST(AppSettings, ComputeQuickAddDeadline) {
    QDateTime now(QDate(2025, 10, 30), QTime(12, 0));
    using D = AppSettings::DefaultDeadline;

    AppSettings::setDefaultDeadline(D::Today);
    auto t1 = AppSettings::computeQuickAddDeadline(now);
    EXPECT_EQ(t1.date(), QDate(2025, 10, 30));

    AppSettings::setDefaultDeadline(D::Tomorrow);
    auto t2 = AppSettings::computeQuickAddDeadline(now);
    EXPECT_EQ(t2.date(), QDate(2025, 10, 31));

    AppSettings::setDefaultDeadline(D::In3Days);
    auto t3 = AppSettings::computeQuickAddDeadline(now);
    EXPECT_EQ(t3.date(), QDate(2025, 11, 2));

    AppSettings::setDefaultDeadline(D::InWeek);
    auto t4 = AppSettings::computeQuickAddDeadline(now);
    EXPECT_EQ(t4.date(), QDate(2025, 11, 6));

    AppSettings::setDefaultDeadline(D::None);
    auto t5 = AppSettings::computeQuickAddDeadline(now);
    EXPECT_FALSE(t5.isValid());
}

TEST(AppSettings, UserStatusAndEmoji) {
    using S = AppSettings::UserStatus;

    AppSettings::setUserStatus(S::Active);
    EXPECT_EQ(AppSettings::userStatus(), S::Active);
    EXPECT_EQ(AppSettings::userStatusEmoji(), "🟢");

    AppSettings::setUserStatus(S::Busy);
    EXPECT_EQ(AppSettings::userStatus(), S::Busy);
    EXPECT_EQ(AppSettings::userStatusEmoji(), "🔴");

    AppSettings::setUserStatus(S::Away);
    EXPECT_EQ(AppSettings::userStatus(), S::Away);
    EXPECT_EQ(AppSettings::userStatusEmoji(), "🟡");

    EXPECT_EQ(AppSettings::userStatusName().isEmpty(), false);
}
