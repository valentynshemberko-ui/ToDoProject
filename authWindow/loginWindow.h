#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H
#include <QDialog>
#include <string>

QT_BEGIN_NAMESPACE
namespace Ui {
    class LoginWindow;
}
QT_END_NAMESPACE

class LoginWindow : public QDialog {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

    void setStatus(const QString &text, bool isError = false);

    signals:
        void loginRequested(const QString &username, const QString &password);
    void registerRequested();

    private slots:
        void on_btnLogin_clicked();
    void on_btnRegister_clicked();
    void on_checkShowPassword_stateChanged(int state);
    void on_textChanged();

private:
    Ui::LoginWindow *ui;
};

#endif //LOGINWINDOW_H
