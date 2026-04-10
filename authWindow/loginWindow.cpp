#include "loginWindow.h"
#include "ui_loginWindow.h"
using namespace std;

LoginWindow::LoginWindow(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
    setWindowTitle("Login");

    ui->labelStatus->clear();
    ui->lineEditPassword->setEchoMode(QLineEdit::Password);

    connect(ui->btnLogin, &QPushButton::clicked, this, &LoginWindow::on_btnLogin_clicked);
    connect(ui->checkShowPassword, &QCheckBox::stateChanged, this, &LoginWindow::on_checkShowPassword_stateChanged);
    connect(ui->lineEditUsername, &QLineEdit::textChanged, this, &LoginWindow::on_textChanged);
    connect(ui->lineEditPassword, &QLineEdit::textChanged, this, &LoginWindow::on_textChanged);
}

LoginWindow::~LoginWindow() {
    delete ui;
}

void LoginWindow::on_btnLogin_clicked() {
    const QString user = ui->lineEditUsername->text().trimmed();
    const QString pass = ui->lineEditPassword->text();

    if (user.isEmpty() || pass.isEmpty()) {
        setStatus("Username and password cannot be empty!", true);
        return;
    }

    emit loginRequested(user, pass);

}

void LoginWindow::on_btnRegister_clicked() {
    emit registerRequested();
}

void LoginWindow::on_checkShowPassword_stateChanged(int state) {
    ui->lineEditPassword->setEchoMode(
        state == Qt::Checked ? QLineEdit::Normal : QLineEdit::Password
    );
}

void LoginWindow::setStatus(const QString &text, bool isError) {
    ui->labelStatus->setText(text);
    ui->labelStatus->setStyleSheet(isError ? "color: red;" : "color: green;");
}

void LoginWindow::on_textChanged() {
    ui->labelStatus->clear();
}