#include "authManager.h"
#include "../authWindow/loginWindow.h"
#include "../authWindow/registerWindow.h"
#include "../logger/globalLogger.h"
#include "../databaseManager/accountRepository.h"
#include <QTimer>

using std::optional;
using std::nullopt;
using std::string;

AuthManager::AuthManager(AccountRepository& repository)
    : repo(repository)
{
    logger.debug("Initializing AuthManager (Qt GUI mode)");
    repo.initTable();

    auto all = repo.getAllAccounts();
    if (all.empty()) {
        logger.warn("No accounts found. Creating default admin...");
        repo.addAccount("admin", std::hash<string>{}("admin"));
        logger.info("Default account 'admin' created.");
    }
}

optional<QString> AuthManager::authenticate(QWidget *parent) {
    LoginWindow login(parent);
    optional<QString> loggedUser = nullopt;

    QObject::connect(&login, &LoginWindow::loginRequested,
                     [&](const QString &qUser, const QString &qPass) {
        handleLoginAttempt(&login, loggedUser, qUser, qPass);
    });

    QObject::connect(&login, &LoginWindow::registerRequested, [&]() {
        openRegistrationWindow(&login);
    });

    login.exec();
    return loggedUser;
}

void AuthManager::handleLoginAttempt(LoginWindow* login, optional<QString>& loggedUser, const QString& qUser, const QString& qPass) {
    const string user = qUser.toStdString();
    const string pass = qPass.toStdString();

    if (user.empty() || pass.empty()) {
        login->setStatus("Username and password cannot be empty!", true);
        return;
    }

    size_t h = std::hash<string>{}(pass);
    auto accOpt = repo.getAccount(user);

    if (accOpt.has_value() && accOpt->getPasswordHash() == h) {
        login->setStatus("Welcome, " + qUser + "!", false);
        logger.info("Login success: " + user);
        loggedUser = qUser;
        QTimer::singleShot(400, login, &QDialog::accept);
    } else {
        login->setStatus("Incorrect username or password", true);
        logger.warn("Login failed for user: " + user);
    }
}

void AuthManager::openRegistrationWindow(LoginWindow* login) {
    RegisterWindow reg(login);

    QObject::connect(&reg, &RegisterWindow::registerRequested,
                     [&](const QString &qUser, const QString &qPass) {
        handleRegistrationAttempt(&reg, qUser, qPass);
    });

    reg.exec();
}

void AuthManager::handleRegistrationAttempt(RegisterWindow* reg, const QString& qUser, const QString& qPass) {
    const string user = qUser.toStdString();
    const string pass = qPass.toStdString();

    if (user.empty() || pass.empty()) {
        reg->setStatus("Username and password cannot be empty!", true);
        logger.warn("Empty username or password during registration attempt.");
        return;
    }
    if (user.length() < 3) {
        reg->setStatus("Username must be at least 3 characters long.", true);
        logger.warn("Too short username during registration: " + user);
        return;
    }
    if (pass.length() < 4) {
        reg->setStatus("Password must be at least 4 characters long.", true);
        logger.warn("Too short password for user: " + user);
        return;
    }
    if (repo.accountExists(user)) {
        reg->setStatus("This username already exists!", true);
        logger.warn("Registration failed: username already exists (" + user + ")");
        return;
    }

    size_t h = std::hash<string>{}(pass);
    bool ok = repo.addAccount(user, h);

    if (ok) {
        reg->setStatus("Account created: " + qUser, false);
        logger.info("New account registered successfully: " + user);
        QTimer::singleShot(600, reg, &QDialog::accept);
    } else {
        reg->setStatus("Error creating account in database!", true);
        logger.error("Account registration failed in DB: " + user);
    }
}