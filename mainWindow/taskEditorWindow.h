#ifndef TASKEDITORWINDOW_H
#define TASKEDITORWINDOW_H

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateEdit>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include "../tasks/task.h"
#include "../windowEdit/framelessWindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TaskEditorWindow; }
QT_END_NAMESPACE

class TaskEditorWindow : public FramelessDialog {
    Q_OBJECT

public:
    explicit TaskEditorWindow(QWidget *parent = nullptr);
    ~TaskEditorWindow();

    Task getTask() const;
    void setTask(const Task& task);

    private slots:
        void saveTask();

private:
    Ui::TaskEditorWindow *ui;
    void setupTitleBar();
};


#endif //TASKEDITORWINDOW_H