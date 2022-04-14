
// MBRClientUpdateDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "MBRClientHttpUpdate.h"

// CMBRClientUpdateDlg 对话框
class CMBRClientUpdateDlg : public CDialogEx
{
// 构造
public:
	CMBRClientUpdateDlg(CWnd* pParent = NULL);	// 标准构造函数
    ~CMBRClientUpdateDlg();
// 对话框数据
	enum { IDD = IDD_MBRCLIENTUPDATE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg LRESULT DownloadProgress(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    CProgressCtrl m_proDownData;
    CButton m_btnCancel;
    double m_dFileSize;
    size_t dataSize;
    CMBRClientHttpUpdate m_clsMBRClientHttpUpdate;
};
