#ifndef REGISTER_H
#define REGISTER_H

#include <QMainWindow>
#include <QDialog>
#include"regist2.h"
#include "Message.hpp"


namespace Ui {
class regist;
}

class regist : public QDialog
{
    Q_OBJECT

public:
    explicit regist(QWidget *parent = 0);
    ~regist();

private slots:
    void on_nextBtn_clicked();
    void receiveData_Register(BYTE response);

private:
    Ui::regist *ui;
    regist2 *r2;
    QString user;
};

#endif // REGISTER_H
