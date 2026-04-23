#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "../authWindow/registerWindow.h"

TEST(RegisterWindowQt, EmitsRegisterRequestedWhenValid) {
    RegisterWindow w;
    QSignalSpy spy(&w, SIGNAL(registerRequested(QString,QString)));

    auto user  = w.findChild<QLineEdit*>("lineEditUsername");
    auto pass1 = w.findChild<QLineEdit*>("lineEditPassword");
    auto pass2 = w.findChild<QLineEdit*>("lineEditConfirmPassword");
    auto btn   = w.findChild<QPushButton*>("btnRegister");

    ASSERT_TRUE(user);
    ASSERT_TRUE(pass1);
    ASSERT_TRUE(pass2);
    ASSERT_TRUE(btn);

    user->setText("denys");
    pass1->setText("abcd1234");
    pass2->setText("abcd1234");

    QCoreApplication::processEvents();
    btn->click();

    QCoreApplication::processEvents();
    EXPECT_GE(spy.count(), 1) << "registerRequested() should be emitted once when input is valid.";
}

TEST(RegisterWindowQt, EmptyFieldsShowErrorStatus) {
    RegisterWindow w;
    auto user = w.findChild<QLineEdit*>("lineEditUsername");
    auto pass1 = w.findChild<QLineEdit*>("lineEditPassword");
    auto pass2 = w.findChild<QLineEdit*>("lineEditConfirmPassword");
    auto btn   = w.findChild<QPushButton*>("btnRegister");
    auto lbl   = w.findChild<QLabel*>("labelStatus");
    ASSERT_TRUE(user); ASSERT_TRUE(pass1); ASSERT_TRUE(pass2); ASSERT_TRUE(btn); ASSERT_TRUE(lbl);

    user->setText("");
    pass1->setText("");
    pass2->setText("");
    btn->click();

    EXPECT_FALSE(lbl->text().isEmpty());
    EXPECT_TRUE(lbl->text().contains("filled", Qt::CaseInsensitive));
    EXPECT_TRUE(lbl->styleSheet().contains("red"));
}

TEST(RegisterWindowQt, PasswordsDoNotMatchShowRedError) {
    RegisterWindow w;
    auto user = w.findChild<QLineEdit*>("lineEditUsername");
    auto pass1 = w.findChild<QLineEdit*>("lineEditPassword");
    auto pass2 = w.findChild<QLineEdit*>("lineEditConfirmPassword");
    auto btn   = w.findChild<QPushButton*>("btnRegister");
    auto lbl   = w.findChild<QLabel*>("labelStatus");
    ASSERT_TRUE(user); ASSERT_TRUE(pass1); ASSERT_TRUE(pass2); ASSERT_TRUE(btn); ASSERT_TRUE(lbl);

    user->setText("denys");
    pass1->setText("abc");
    pass2->setText("xyz");

    btn->click();

    EXPECT_FALSE(lbl->text().isEmpty());
    EXPECT_TRUE(lbl->text().contains("Passwords", Qt::CaseInsensitive));
    EXPECT_TRUE(lbl->styleSheet().contains("red"));
}

TEST(RegisterWindowQt, ValidPasswordsShowGreenStatus) {
    RegisterWindow w;
    auto user = w.findChild<QLineEdit*>("lineEditUsername");
    auto pass1 = w.findChild<QLineEdit*>("lineEditPassword");
    auto pass2 = w.findChild<QLineEdit*>("lineEditConfirmPassword");
    auto btn   = w.findChild<QPushButton*>("btnRegister");
    auto lbl   = w.findChild<QLabel*>("labelStatus");
    ASSERT_TRUE(user); ASSERT_TRUE(pass1); ASSERT_TRUE(pass2); ASSERT_TRUE(btn); ASSERT_TRUE(lbl);

    user->setText("newuser");
    pass1->setText("samepass");
    pass2->setText("samepass");

    btn->click();
    EXPECT_TRUE(lbl->text().isEmpty() || lbl->styleSheet().contains("green") || lbl->styleSheet().isEmpty());
}

TEST(RegisterWindowQt, StatusClearsWhenTyping) {
    RegisterWindow w;
    auto user = w.findChild<QLineEdit*>("lineEditUsername");
    auto pass1 = w.findChild<QLineEdit*>("lineEditPassword");
    auto pass2 = w.findChild<QLineEdit*>("lineEditConfirmPassword");
    auto lbl   = w.findChild<QLabel*>("labelStatus");
    auto btn   = w.findChild<QPushButton*>("btnRegister");
    ASSERT_TRUE(user); ASSERT_TRUE(pass1); ASSERT_TRUE(pass2); ASSERT_TRUE(btn); ASSERT_TRUE(lbl);

    user->setText("denys");
    pass1->setText("a");
    pass2->setText("b");
    btn->click();
    EXPECT_FALSE(lbl->text().isEmpty());

    pass2->setText("ab");
    QCoreApplication::processEvents();
    EXPECT_TRUE(lbl->text().isEmpty()) << "Status label should clear on text change.";
}

TEST(RegisterWindowQt, SetStatusAppliesCorrectColor) {
    RegisterWindow w;
    auto lbl = w.findChild<QLabel*>("labelStatus");
    ASSERT_TRUE(lbl);

    w.setStatus("All good", false);
    EXPECT_EQ(lbl->text(), "All good");
    EXPECT_TRUE(lbl->styleSheet().contains("green"));

    w.setStatus("Error!", true);
    EXPECT_EQ(lbl->text(), "Error!");
    EXPECT_TRUE(lbl->styleSheet().contains("red"));
}
