#ifndef LOGINSYSTEM_H
#define LOGINSYSTEM_H

#include <QWidget>
//#include<QQuickWidget>
#include <QLabel>
#include <define.h>
#include<QDebug>
#include<QDesktopWidget>
#include<QPushButton>
#include<QSignalMapper>
#include<QVector>
#include<QButtonGroup>
#include<QTextCursor>
#include<QSqlDatabase>
#include<QSqlQuery>
#include<QMessageBox>

#define BSPACE 0x2
#define TAB 0X3
#define CAPS 0x4
#define ENTER 0x5
#define SHIFT 0x6
#define SHIFT2 0x7

namespace Ui {
class LoginSystem;
}


class LoginSystem : public QWidget
{
    Q_OBJECT

public:
    explicit LoginSystem(QWidget *parent = nullptr);
    ~LoginSystem();
    void SetControlStyleSheet();
    void SetControlStyleSheet_600P();
    void ResizeControl(int width,int height);
    void ResizeControl_600P(int width,int height);
    void CreatKeyBoard();
    bool IsSpecialButton(char but);
    void SetEdit(char but);

    int showWarn(QWidget *parent, const QString &title, const QString &content);

    void mousePressEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent *k);
private slots:
    void on_pushButton_Login_clicked();

    void on_pushButton_Cancel_clicked();

    void buttonNumslot(int);

signals:
    void ChangToUserWin(int iLoginType);

private:
    Ui::LoginSystem *ui;
    QVector<char>CapKey;
    QVector<char>ComKey;
    QList<QPushButton *> allButtons;
   // QSignalMapper *signalMapper;
    QButtonGroup *m_buGroup;
    QPushButton*  keyBoardButton[54];
    int keyButtonCount;

    bool    bCapsOn;
    int     ShiftIndex;

};

#endif // LOGINSYSTEM_H
