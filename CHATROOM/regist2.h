#ifndef REGIST2_H
#define REGIST2_H

#include <QDialog>
#include "Message.hpp"

namespace Ui {
class regist2;
}

class regist2 : public QDialog
{
    Q_OBJECT

public:
    explicit regist2(QString user_name);
    ~regist2();

private slots:
    void on_conBtn_clicked();
    void receiveDate_Register1(BYTE response);

private:
    Ui::regist2 *ui;
    QString user;
};

#endif // REGIST2_H
