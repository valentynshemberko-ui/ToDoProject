#include "registerWindow.h"
#include "ui_registerWindow.h"
using namespace std;

RegisterWindow::RegisterWindow(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::RegisterWindow)
{
    ui->setupUi(this);
    setWindowTitle("Registration");

    ui->labelStatus->clear();
    connect(ui->lineEditUsername, &QLineEdit::textChanged, this, &RegisterWindow::on_textChanged);
    connect(ui->lineEditPassword, &QLineEdit::textChanged, this, &RegisterWindow::on_textChanged);
    connect(ui->lineEditConfirmPassword, &QLineEdit::textChanged, this, &RegisterWindow::on_textChanged);

}

RegisterWindow::~RegisterWindow() {
    delete ui;
}

void RegisterWindow::on_btnRegister_clicked() {
    const QString user  = ui->lineEditUsername->text().trimmed();
    const QString pass1 = ui->lineEditPassword->text();
    const QString pass2 = ui->lineEditConfirmPassword->text();

    if (user.isEmpty() || pass1.isEmpty() || pass2.isEmpty()) {
        setStatus("All fields must be filled in!", true);
        return;
    }
    if (pass1 != pass2) {
        setStatus("Passwords do not match!", true);
        return;
    }
    emit registerRequested(user, pass1);
}

void RegisterWindow::setStatus(const QString &text, bool isError) {
    ui->labelStatus->setText(text);
    ui->labelStatus->setStyleSheet(isError ? "color: red;" : "color: green;");
}

void RegisterWindow::on_textChanged(){
    ui->labelStatus->clear();
}
