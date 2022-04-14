#include "datetimesetwidget.h"
#include "ui_datetimesetwidget.h"

DateTimeSetWidget::DateTimeSetWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DateTimeSetWidget)
{
    ui->setupUi(this);
}

DateTimeSetWidget::~DateTimeSetWidget()
{
    delete ui;
}

void DateTimeSetWidget::on_pushButtonOk_clicked()
{
    accept();
}

void DateTimeSetWidget::on_pushButtonCancel_clicked()
{
    reject();
}

void DateTimeSetWidget::SetDateTime(const QDateTime &dt)
{
    ui->dateEdit->setDate(dt.date());
    ui->timeEdit->setTime(dt.time());
}

QDateTime DateTimeSetWidget::GetDateTime()
{
    QDateTime dt;
    dt.setDate(ui->dateEdit->date());
    dt.setTime(ui->timeEdit->time());
    return dt;
}
