#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QDialog>
#include <string>

QT_BEGIN_NAMESPACE
namespace Ui { class RegisterWindow; }
QT_END_NAMESPACE

class RegisterWindow : public QDialog {
    Q_OBJECT

public:
    explicit RegisterWindow(QWidget *parent = nullptr);
    ~RegisterWindow();

    void setStatus(const QString &text, bool isError = false);

    signals:
        void registerRequested(const QString &username, const QString &password);

    private slots:
        void on_btnRegister_clicked();
        void on_textChanged();

private:
    Ui::RegisterWindow *ui;
};


#endif //REGISTERWINDOW_H
