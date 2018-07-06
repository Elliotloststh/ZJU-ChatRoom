#ifndef CHAT_RECORD_H
#define CHAT_RECORD_H

#include <QDialog>

namespace Ui {
class chat_record;
}

class chat_record : public QDialog
{
    Q_OBJECT

public:
    explicit chat_record(QWidget *parent = 0);
    ~chat_record();

private slots:
    void on_select_time_clicked();

    void on_quit_clicked();

    void on_calendarWidget_clicked(const QDate &date);

private:
    Ui::chat_record *ui;
};

#endif // CHAT_RECORD_H
