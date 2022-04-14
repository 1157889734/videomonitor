#ifndef DATETIMESETWIDGET_H
#define DATETIMESETWIDGET_H

#include <QDialog>
#include <QWidget>

namespace Ui {
class DateTimeSetWidget;
}

class DateTimeSetWidget : public QDialog
{
    Q_OBJECT

public:
    explicit DateTimeSetWidget(QWidget *parent = 0);
    ~DateTimeSetWidget();

    void SetDateTime(const QDateTime &dt);
    QDateTime GetDateTime();
private slots:
    void on_pushButtonOk_clicked();

    void on_pushButtonCancel_clicked();

private:
    Ui::DateTimeSetWidget *ui;
};

#endif // DATETIMESETWIDGET_H
