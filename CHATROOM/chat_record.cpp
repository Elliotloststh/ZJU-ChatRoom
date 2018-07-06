#include "chat_record.h"
#include "ui_chat_record.h"
#include <QDebug>
static bool flag = false;
chat_record::chat_record(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::chat_record)
{
    ui->setupUi(this);
    setWindowTitle("聊天记录");
    ui->calendarWidget->hide();
}

chat_record::~chat_record()
{
    delete ui;
}

void chat_record::on_select_time_clicked()
{
    if(flag == false){
        ui->calendarWidget->show();
        flag = true;
    }
    else{
        ui->calendarWidget->hide();
        flag = false;
    }
}

void chat_record::on_quit_clicked()
{
    close();
}

void chat_record::on_calendarWidget_clicked(const QDate &date)
{
    QString selected_date = date.toString("yyyy-MM-dd");
    qDebug() << selected_date;
    ui->calendarWidget->hide();
    flag = false;
}
