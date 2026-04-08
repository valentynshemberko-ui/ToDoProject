#ifndef AUTHMANAGER_H
#define AUTHMANAGER_H

#include <optional>
#include <QString>
#include "../authWindow/loginWindow.h"
#include "../authWindow/registerWindow.h"

class AccountRepository;
class QWidget;

class AuthManager {
public:
    explicit AuthManager(AccountRepository& repository);
    std::optional<QString> authenticate(QWidget *parent = nullptr);

private:
    void handleLoginAttempt(LoginWindow* login, std::optional<QString>& loggedUser, const QString& qUser, const QString& qPass);
    void handleRegistrationAttempt(RegisterWindow* reg, const QString& qUser, const QString& qPass);
    void openRegistrationWindow(LoginWindow* login);
    AccountRepository& repo;
};

#endif // AUTHMANAGER_H
