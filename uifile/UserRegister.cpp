#include "UserRegister.h"
#include "ui_UserRegister.h"
#include<QDebug>
#include "mainsdk.h"
#include "Keyboard.h"
#include "UserMonitor.h"
CUserRegister::CUserRegister(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CUserRegister),
    FocusIndex(0)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    ui->widgetTitle->move(0,0);
    ui->widgetTitle->resize(USER_REGISYER_WIN_WIDTH,USER_REGISYER_WIN_HEIGHT);
    ui->radioButtonAdminUser->setChecked(true);
    ui->lineEditUser->installEventFilter(this);
    ui->lineEditPassword->installEventFilter(this);
    ui->lineEditSurePassword->installEventFilter(this);

    ui->lineEditPassword->setEchoMode(QLineEdit::Password);
    ui->lineEditSurePassword->setEchoMode(QLineEdit::Password);

}

CUserRegister::~CUserRegister()
{
    delete ui;
}

void CUserRegister::showEvent(QShowEvent *)
{
    int iLoginType = MAIN_GetSdk()->GetUserStyle();
    if(E_USER_INFO_SUPER_ADMIN == iLoginType)
    {
        ui->radioButtonAdminUser->show();
        //ui->radioButtonAdminUser->setChecked(false);
        //ui->radioButtonOperatorUser->setChecked(true);
    }
    else if(E_USER_INFO_ADMIN == iLoginType)
    {
        ui->radioButtonAdminUser->hide();
        ui->radioButtonAdminUser->setChecked(false);
        ui->radioButtonOperatorUser->setChecked(true);
    }
}

void CUserRegister::InputKeySlots(char key)
{
    //backspace
    if(key==BSPACE)
    {
        if(FocusIndex==1)
        {

            if(!ui->lineEditUser->selectedText().isEmpty())
            {
                ui->lineEditUser->del();
            }
            else
            {
                ui->lineEditUser->backspace();
            }

        }
        else if(FocusIndex==2)
        {
            if(!ui->lineEditPassword->selectedText().isEmpty())
            {
                ui->lineEditPassword->del();
            }
            else
            {
                ui->lineEditPassword->backspace();
            }

        }
        else if(FocusIndex==3)
        {
            if(!ui->lineEditSurePassword->selectedText().isEmpty())
            {
                ui->lineEditSurePassword->del();
            }
            else
            {
                ui->lineEditSurePassword->backspace();
            }
        }
    }
    else if(key == TAB)
    {
        //tab
        if(FocusIndex==1)//输入框1焦点)
        {
            ui->lineEditPassword->setFocus();
            FocusIndex=2;
        }
        else if(FocusIndex==2)
        {
            ui->lineEditSurePassword->setFocus();
            FocusIndex=3;
        }
        else if(FocusIndex==3)
        {
            ui->lineEditUser->setFocus();
            FocusIndex=1;
        }
    }
    else if(key == ENTER)
    {
        // enter
        on_pushButtonSave_clicked();
    }
    else
    {
        if(FocusIndex==1)
        {
            ui->lineEditUser->insert(QString( key));

        }
        else if(FocusIndex==2)
        {
            ui->lineEditPassword->insert(QString( key));

        }
        else if(FocusIndex==3)
        {
            ui->lineEditSurePassword->insert(QString( key));
        }
    }

 }


 bool CUserRegister::eventFilter(QObject *obj, QEvent *e)
 {
     if(obj == ui->lineEditUser && e->type() == QEvent::MouseButtonPress)
     {
           FocusIndex=1;
           emit ShowKeyboardSignals();
           return true;
     }
     else if(obj == ui->lineEditPassword && e->type() == QEvent::MouseButtonPress)
     {
           FocusIndex=2;
           emit ShowKeyboardSignals();
           return true;
     }
     else if(obj == ui->lineEditSurePassword &&e->type() == QEvent::MouseButtonPress)
     {
          FocusIndex=3;
          emit ShowKeyboardSignals();
          return true;
     }
     return QWidget::eventFilter(obj,e);        //返回默认的事件过滤器
 }



void CUserRegister::on_pushButtonSave_clicked()
{
    if(ui->lineEditUser->text().isEmpty() ||
        ui->lineEditPassword->text().isEmpty() ||
        ui->lineEditSurePassword->text().isEmpty())
    {

        emit RegisterUserInfoSignals(QVariant(), -1);
        return ;
    }

    RegisterUserInfo  m_UserInfo;
    m_UserInfo.UserName =ui->lineEditUser->text();
    m_UserInfo.PassWord =ui->lineEditPassword->text();
    if(m_UserInfo.PassWord != ui->lineEditSurePassword->text())
    {
        emit RegisterUserInfoSignals(QVariant(), -2);
        return;
    }

    if(ui->radioButtonAdminUser->isChecked())
    {
        m_UserInfo.iOperator = E_USER_INFO_ADMIN;
    }
    else if(ui->radioButtonOperatorUser->isChecked())
    {
        m_UserInfo.iOperator = E_USER_INFO_OPER;
    }

    ui->lineEditUser ->clear();
    ui->lineEditPassword->clear();
    ui->lineEditSurePassword->clear();
    this->hide();

    QVariant Variant;
    Variant.setValue(m_UserInfo);
    emit RegisterUserInfoSignals(Variant, 0);
}

void CUserRegister::on_pushButtonCancel_clicked()
{
    ui->lineEditUser ->clear();
    ui->lineEditPassword->clear();
    ui->lineEditSurePassword->clear();
    this->hide();
    emit RegisterUserInfoSignals(QVariant(), -3);
}
