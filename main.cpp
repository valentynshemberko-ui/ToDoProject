#include "mainWindow/mainwindow.h"
#include "databaseManager/accountRepository.h"
#include "logger/globalLogger.h"
#include "databaseManager/TaskRepository.h"
#include "tasks/TaskManager.h"
#include <QApplication>
//Uncomment if there is work with the terminal
/*#ifdef _WIN32
#include <windows.h>
void enableANSI() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}
#endif*/

int main(int argc, char *argv[]) {
/*#ifdef _WIN32
    enableANSI();
#endif*/
    QApplication app(argc, argv);

    app.setWindowIcon(QIcon(":/resources/icons/to-do.png"));

    DatabaseManager db("todo.db");
    AccountRepository accRepo(db);
    AuthManager auth(accRepo);

    auto loggedUser = auth.authenticate(nullptr);
    //optional<string> loggedUser = "admin";  //DEFAULT ACCOUT

    if (!loggedUser.has_value()) {
        return 0;
    }

    TaskRepository taskRepo(db);
    TaskManager taskManager(taskRepo);
    taskManager.setCurrentUser(loggedUser.value().toStdString());

    MainWindow mainWin(taskManager);
    mainWin.show();

    return app.exec();
}