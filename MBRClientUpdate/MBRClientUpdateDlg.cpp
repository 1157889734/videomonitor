
// MBRClientUpdateDlg.cpp : 实现文件
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
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CMBRClientUpdateDlg 对话框


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


// CMBRClientUpdateDlg 消息处理程序

BOOL CMBRClientUpdateDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
    SetWindowPos(NULL, 0, 0, 800,150, SWP_NOZORDER);


    CRect stcnPro(150,15,650,55);
    CRect stcnBtn(350,65,450,105);
    m_proDownData.GetSafeHwnd();
    m_proDownData.SetWindowPos(NULL, stcnPro.left, stcnPro.top, stcnPro.Width(),stcnPro.Height(), SWP_NOZORDER);
    m_btnCancel.SetWindowPos(NULL, stcnBtn.left, stcnBtn.top, stcnBtn.Width(),stcnBtn.Height(), SWP_NOZORDER);


    if(!AEYEDBOPERATOR->openDBFile()) 
    {
        MessageBoxA(GetSafeHwnd(),"数据库打开失败","提示",MB_OK);
        return 0;
    }
    m_dFileSize = m_clsMBRClientHttpUpdate.GetFileTotalSize(GetSafeHwnd());

    m_proDownData.SetRange(0,m_dFileSize);
    m_clsMBRClientHttpUpdate.VersionUpdate();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMBRClientUpdateDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMBRClientUpdateDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMBRClientUpdateDlg::OnBnClickedOk()
{
    // TODO: 在此添加控件通知处理程序代码
    CDialogEx::OnOK();
}


void CMBRClientUpdateDlg::OnBnClickedCancel()
{
    // TODO: 在此添加控件通知处理程序代码
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