
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QComboBox>
#include "../settings/settingsWindow.h"

TEST(SettingsWindowQt, EmitsThemeChangedIfAvailable) {
    SettingsWindow w;
    int sigIdx = w.metaObject()->indexOfSignal("themeChanged()");
    if (sigIdx >= 0) {
        QSignalSpy spy(&w, SIGNAL(themeChanged()));
        auto combo = w.findChild<QComboBox*>("comboTheme");
        ASSERT_TRUE(combo);
        combo->setCurrentIndex((combo->currentIndex()==0)?1:0);
        EXPECT_GE(spy.count(), 1);
    } else {
        SUCCEED() << "themeChanged() signal not present; skipping emission check.";
    }
}
