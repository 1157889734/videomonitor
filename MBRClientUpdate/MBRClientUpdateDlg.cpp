
// MBRClientUpdateDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MBRClientUpdate.h"
#include "MBRClientUpdateDlg.h"
#include "afxdialogex.h"
#include "AEyeDBOperator.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_DOWNLOAD_PROGRESS  WM_USER + 10310
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMBRClientUpdateDlg �Ի���


CMBRClientUpdateDlg::CMBRClientUpdateDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMBRClientUpdateDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_dFileSize = 0;
    dataSize = 0;
}

 CMBRClientUpdateDlg::~CMBRClientUpdateDlg()
 {
     AEYEDBOPERATOR->ReleaseInstance();
 }

void CMBRClientUpdateDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_PROGRESS1, m_proDownData);
    DDX_Control(pDX, IDCANCEL, m_btnCancel);
}

BEGIN_MESSAGE_MAP(CMBRClientUpdateDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDOK, &CMBRClientUpdateDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CMBRClientUpdateDlg::OnBnClickedCancel)
    ON_MESSAGE(WM_DOWNLOAD_PROGRESS,&CMBRClientUpdateDlg::DownloadProgress)
END_MESSAGE_MAP()


// CMBRClientUpdateDlg ��Ϣ�������

BOOL CMBRClientUpdateDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
    SetWindowPos(NULL, 0, 0, 800,150, SWP_NOZORDER);


    CRect stcnPro(150,15,650,55);
    CRect stcnBtn(350,65,450,105);
    m_proDownData.GetSafeHwnd();
    m_proDownData.SetWindowPos(NULL, stcnPro.left, stcnPro.top, stcnPro.Width(),stcnPro.Height(), SWP_NOZORDER);
    m_btnCancel.SetWindowPos(NULL, stcnBtn.left, stcnBtn.top, stcnBtn.Width(),stcnBtn.Height(), SWP_NOZORDER);


    if(!AEYEDBOPERATOR->openDBFile()) 
    {
        MessageBoxA(GetSafeHwnd(),"���ݿ��ʧ��","��ʾ",MB_OK);
        return 0;
    }
    m_dFileSize = m_clsMBRClientHttpUpdate.GetFileTotalSize(GetSafeHwnd());

    m_proDownData.SetRange(0,m_dFileSize);
    m_clsMBRClientHttpUpdate.VersionUpdate();

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CMBRClientUpdateDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CMBRClientUpdateDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CMBRClientUpdateDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMBRClientUpdateDlg::OnBnClickedOk()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    CDialogEx::OnOK();
}


void CMBRClientUpdateDlg::OnBnClickedCancel()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    CDialogEx::OnCancel();
}

LRESULT CMBRClientUpdateDlg::DownloadProgress(WPARAM wParam, LPARAM lParam)
{
    dataSize += (size_t)wParam;
   
    m_proDownData.SetPos(dataSize);

    if(dataSize>=m_dFileSize)
    {
         CDialogEx::OnCancel();
    }

    m_proDownData.UpdateWindow();
    return 1;
}