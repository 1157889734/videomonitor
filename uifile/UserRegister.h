#ifndef USERREGISTER_H
#define USERREGISTER_H

#include <QDialog>
#include<define.h>
#include<QVariant>

typedef struct tagRegisterUserInfo
{
    QString UserName;
    QString PassWord;
    int     iOperator;

}RegisterUserInfo;

Q_DECLARE_METATYPE(RegisterUserInfo)

namespace Ui {
class CUserRegister;
}

class CUserRegister : public QDialog
{
    Q_OBJECT

public:
    explicit CUserRegister(QWidget *parent = nullptr);
    ~CUserRegister();
     bool eventFilter(QObject *obj, QEvent *e);


protected:
     virtual void showEvent(QShowEvent *);
private slots:
    void on_pushButtonSave_clicked();

    void on_pushButtonCancel_clicked();
    void InputKeySlots(char);

signals:
    void ShowKeyboardSignals(int nShow=1);
    void RegisterUserInfoSignals(QVariant,int);

private:
    Ui::CUserRegister *ui;
    int FocusIndex;
};

#endif // USERREGISTER_H
