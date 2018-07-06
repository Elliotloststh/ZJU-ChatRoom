#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "client.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QString user_name);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void receiveData_Public(string username, string msg, string time);

private:
    Ui::MainWindow *ui;
    const QString& user;

};

#endif // MAINWINDOW_H
