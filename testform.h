#ifndef TESTFORM_H
#define TESTFORM_H

#include <QWidget>

namespace Ui {
class TestForm;
}

class TestForm : public QWidget
{
    Q_OBJECT

public:
    explicit TestForm(QWidget *parent = 0);
    ~TestForm();

    void ResizeControl(int Width,int Height);

    //设置样式表
    void SetControlStyleSheet();

    void InitControl();

private:
    Ui::TestForm *ui;
    int  MainState;
    int  SubState;
    QTimer *timer;
    int LastSelectCarriageIdx;
    int CurSelectCarriageIdx;

    int  m_iCurrentPage;
    int  iVideoFormat;
    bool bSingleVideoWin;
};

#endif // TESTFORM_H
