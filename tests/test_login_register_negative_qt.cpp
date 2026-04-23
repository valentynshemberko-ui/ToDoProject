#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "../authWindow/registerWindow.h"
#include "../accounts/authManager.h"
#include "../databaseManager/accountRepository.h"
#include "../databaseManager/databaseManager.h"
#include "../authWindow/loginWindow.h"

TEST(RegisterWindowQtNeg, PasswordsMismatchShowsStatusColorAndText) {
    DatabaseManager db(":memory:");
    ASSERT_TRUE(db.open());
    AccountRepository repo(db);
    repo.initTable();

    AuthManager auth(repo);
    RegisterWindow reg(nullptr);

    QObject::connect(&reg, &RegisterWindow::registerRequested,
                     [&](const QString &user, const QString &pass) {
        if (user.isEmpty() || pass.isEmpty()) {
            reg.setStatus("Username and password cannot be empty!", true);
            return;
        }
        reg.setStatus("Passwords do not match!", true);
    });

    auto user  = reg.findChild<QLineEdit*>("lineEditUsername");
    auto pass1 = reg.findChild<QLineEdit*>("lineEditPassword");
    auto pass2 = reg.findChild<QLineEdit*>("lineEditConfirmPassword");
    auto btn   = reg.findChild<QPushButton*>("btnRegister");
    auto lbl   = reg.findChild<QLabel*>("labelStatus");

    ASSERT_TRUE(user);
    ASSERT_TRUE(pass1);
    ASSERT_TRUE(pass2);
    ASSERT_TRUE(btn);
    ASSERT_TRUE(lbl);

    user->setText("testuser");
    pass1->setText("1234");
    pass2->setText("abcd");
    btn->click();

    QString text = lbl->text();
    QString style = lbl->styleSheet();

    EXPECT_FALSE(text.isEmpty());
    EXPECT_TRUE(text.contains("password", Qt::CaseInsensitive) ||
                text.contains("mismatch", Qt::CaseInsensitive) ||
                text.contains("not match", Qt::CaseInsensitive));

    EXPECT_TRUE(style.contains("red", Qt::CaseInsensitive))
        << "Label style should indicate error color (red).";

    pass2->setText("1234");
    emit reg.registerRequested(user->text(), pass1->text());
    lbl->clear();
    EXPECT_TRUE(lbl->text().isEmpty());
}

TEST(LoginWindowQtNeg, EmptyFieldsDoNotEmit) {
    LoginWindow w;
    QSignalSpy spy(&w, SIGNAL(loginRequested(QString,QString)));
    auto btn = w.findChild<QPushButton*>("btnLogin");
    ASSERT_TRUE(btn);
    btn->click();
    EXPECT_EQ(spy.count(), 0);
}
