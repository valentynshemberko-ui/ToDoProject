
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include "../authWindow/loginWindow.h"

TEST(LoginWindowQt, EmitsLoginRequested) {
    LoginWindow w;
    QSignalSpy spy(&w, SIGNAL(loginRequested(QString,QString)));
    auto userEdit  = w.findChild<QLineEdit*>("lineEditUsername");
    auto passEdit  = w.findChild<QLineEdit*>("lineEditPassword");
    auto btnLogin  = w.findChild<QPushButton*>("btnLogin");
    ASSERT_TRUE(userEdit); ASSERT_TRUE(passEdit); ASSERT_TRUE(btnLogin);
    userEdit->setText("denys");
    passEdit->setText("qwerty");
    btnLogin->click();
    EXPECT_GE(spy.count(), 1);
    auto args = spy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), "denys");
    EXPECT_EQ(args.at(1).toString(), "qwerty");
}

TEST(LoginWindowQt, ShowPasswordTogglesEchoMode) {
    LoginWindow w;
    auto passEdit  = w.findChild<QLineEdit*>("lineEditPassword");
    auto cbShow    = w.findChild<QCheckBox*>("checkShowPassword");
    ASSERT_TRUE(passEdit); ASSERT_TRUE(cbShow);
    auto initial = passEdit->echoMode();
    cbShow->setChecked(true);
    EXPECT_NE(passEdit->echoMode(), initial);
    cbShow->setChecked(false);
    EXPECT_EQ(passEdit->echoMode(), initial);
}
