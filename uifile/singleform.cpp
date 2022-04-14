#include "singleform.h"
#include "ui_singleform.h"


SingleForm::SingleForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SingleForm)
{
    ui->setupUi(this);

    m_pCMPHandle = nullptr;
    //this->setWindowModality(Qt::ApplicationModal);
    this->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    setStyleSheet("QWidget{background-color: #000;}");
}

SingleForm::~SingleForm()
{
    delete ui;
}

void SingleForm::mousePressEvent(QMouseEvent *event)
{
    emit signalCmd(0);
    Stop();
    setStyleSheet("QWidget{background-color: #000;}");
    this->hide();
}

int SingleForm::Start(const char *rtsp)
{
    Stop();
    QRect rt = this->geometry();
    m_tWndInfo.nVideoWidth = 0;
    m_tWndInfo.nVideoHeight = 0;
    m_tWndInfo.pRenderHandle = NULL;
    m_tWndInfo.nX = rt.x();
    m_tWndInfo.nY = rt.y();
    m_tWndInfo.nWidth = rt.width();
    m_tWndInfo.nHeight = rt.height();
    m_tWndInfo.hWnd = (HWND)ui->label->winId();
    m_pCMPHandle = CMP_Init((&m_tWndInfo)->hWnd, CMP_VDEC_NORMAL);
    CMP_OpenMediaPreview(m_pCMPHandle, rtsp, CMP_TCP);
    return 0;
}

int SingleForm::Stop()
{
    if(m_pCMPHandle == nullptr)
    {
        return 0;
    }
    CMP_CloseMedia(m_pCMPHandle);
    CMP_UnInit(m_pCMPHandle);
    m_pCMPHandle = nullptr;
    return 0;
}
