
#include <gtest/gtest.h>
#include <optional>
#include <QApplication>
#include <QTimer>
#include "../authWindow/loginWindow.h"
#include "../authWindow/registerWindow.h"
#include "../databaseManager/databaseManager.h"
#include "../databaseManager/accountRepository.h"
#include "../accounts/authManager.h"

template <typename T>
T* findActiveWindow() {
    for (QWidget* widget : QApplication::topLevelWidgets()) {
        T* window = qobject_cast<T*>(widget);
        if (window && window->isVisible()) {
            return window;
        }
    }
    return nullptr;
}

struct AuthFx : public ::testing::Test {
    DatabaseManager db = DatabaseManager(":memory:");
    std::unique_ptr<AccountRepository> repo;
    std::unique_ptr<AuthManager> auth;

    void SetUp() override {
        ASSERT_TRUE(db.open());
        repo = std::make_unique<AccountRepository>(db);
        repo->initTable();
        auth = std::make_unique<AuthManager>(*repo);
    }
};

TEST_F(AuthFx, DefaultAdminCreated) {
    auto all = repo->getAllAccounts();
    bool hasAdmin = false;
    for (auto &a : all) if (a.getUsername() == "admin") hasAdmin = true;
    EXPECT_TRUE(hasAdmin);
}

TEST_F(AuthFx, AddAndAuthenticate) {
    EXPECT_TRUE(repo->addAccount("denys", std::hash<std::string>{}("qwerty")));
    auto acc = repo->getAccount("denys");
    ASSERT_TRUE(acc.has_value());
    EXPECT_EQ(acc->getUsername(), "denys");
}

TEST_F(AuthFx, AuthenticateSuccess) {
    repo->addAccount("user_ok", std::hash<std::string>{}("pass123"));

    QTimer::singleShot(100, []() {
        auto loginWin = findActiveWindow<LoginWindow>();
        ASSERT_TRUE(loginWin != nullptr);

        emit loginWin->loginRequested("user_ok", "pass123");
    });

    auto res = auth->authenticate();
    EXPECT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), "user_ok");
}

TEST_F(AuthFx, AuthenticateFailsEmptyFields) {
    QTimer::singleShot(100, []() {
        auto loginWin = findActiveWindow<LoginWindow>();
        ASSERT_TRUE(loginWin != nullptr);

        emit loginWin->loginRequested("", "");

        QTimer::singleShot(100, loginWin, &QDialog::reject);
    });

    auto res = auth->authenticate();
    EXPECT_FALSE(res.has_value());
}

TEST_F(AuthFx, AuthenticateFailsIncorrectPassword) {
    repo->addAccount("user", std::hash<std::string>{}("correct_pass"));

    QTimer::singleShot(100, []() {
        auto loginWin = findActiveWindow<LoginWindow>();
        ASSERT_TRUE(loginWin != nullptr);

        emit loginWin->loginRequested("user", "wrong_pass");
        QTimer::singleShot(100, loginWin, &QDialog::reject); // Закриваємо вручну
    });

    auto res = auth->authenticate();
    EXPECT_FALSE(res.has_value());
}

TEST_F(AuthFx, RegisterSuccess) {
    QTimer::singleShot(100, []() {
        auto loginWin = findActiveWindow<LoginWindow>();
        ASSERT_TRUE(loginWin != nullptr);

        QTimer::singleShot(100, [loginWin]() {
            auto regWin = findActiveWindow<RegisterWindow>();
            ASSERT_TRUE(regWin != nullptr);

            emit regWin->registerRequested("new_user", "new_pass");

            QTimer::singleShot(800, loginWin, &QDialog::reject);
        });

        emit loginWin->registerRequested();
    });

    auto res = auth->authenticate();
    EXPECT_FALSE(res.has_value());
    EXPECT_TRUE(repo->accountExists("new_user"));
}

TEST_F(AuthFx, RegisterFailsEmptyFields) {
    QTimer::singleShot(100, []() {
        auto loginWin = findActiveWindow<LoginWindow>();
        if (loginWin) {
            QTimer::singleShot(100, [loginWin]() {
                auto regWin = findActiveWindow<RegisterWindow>();
                if (regWin) {
                    emit regWin->registerRequested("", "pass");
                    QTimer::singleShot(100, regWin, &QDialog::reject);
                }
                QTimer::singleShot(200, loginWin, &QDialog::reject);
            });
            emit loginWin->registerRequested();
        }
    });

    auth->authenticate();
    EXPECT_FALSE(repo->accountExists(""));
}

TEST_F(AuthFx, RegisterFailsShortUsername) {
    QTimer::singleShot(100, []() {
        auto loginWin = findActiveWindow<LoginWindow>();
        if (loginWin) {
            QTimer::singleShot(100, [loginWin]() {
                auto regWin = findActiveWindow<RegisterWindow>();
                if (regWin) {
                    emit regWin->registerRequested("ab", "pass123");
                    QTimer::singleShot(100, regWin, &QDialog::reject);
                }
                QTimer::singleShot(200, loginWin, &QDialog::reject);
            });
            emit loginWin->registerRequested();
        }
    });

    auth->authenticate();
    EXPECT_FALSE(repo->accountExists("ab"));
}

TEST_F(AuthFx, RegisterFailsShortPassword) {
    QTimer::singleShot(100, []() {
        auto loginWin = findActiveWindow<LoginWindow>();
        if (loginWin) {
            QTimer::singleShot(100, [loginWin]() {
                auto regWin = findActiveWindow<RegisterWindow>();
                if (regWin) {
                    emit regWin->registerRequested("user_1", "123");
                    QTimer::singleShot(100, regWin, &QDialog::reject);
                }
                QTimer::singleShot(200, loginWin, &QDialog::reject);
            });
            emit loginWin->registerRequested();
        }
    });

    auth->authenticate();
    EXPECT_FALSE(repo->accountExists("user_1"));
}

TEST_F(AuthFx, RegisterFailsUserAlreadyExists) {
    repo->addAccount("existing_user", std::hash<std::string>{}("pass123"));

    QTimer::singleShot(100, []() {
        auto loginWin = findActiveWindow<LoginWindow>();
        if (loginWin) {
            QTimer::singleShot(100, [loginWin]() {
                auto regWin = findActiveWindow<RegisterWindow>();
                if (regWin) {
                    emit regWin->registerRequested("existing_user", "another_pass");
                    QTimer::singleShot(100, regWin, &QDialog::reject);
                }
                QTimer::singleShot(200, loginWin, &QDialog::reject);
            });
            emit loginWin->registerRequested();
        }
    });

    auth->authenticate();
    auto acc = repo->getAccount("existing_user");
    EXPECT_EQ(acc->getPasswordHash(), std::hash<std::string>{}("pass123"));
}