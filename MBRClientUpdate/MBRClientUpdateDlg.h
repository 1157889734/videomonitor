
// MBRClientUpdateDlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "MBRClientHttpUpdate.h"

// CMBRClientUpdateDlg �Ի���
class CMBRClientUpdateDlg : public CDialogEx
{
// ����
public:
	CMBRClientUpdateDlg(CWnd* pParent = NULL);	// ��׼���캯��
    ~CMBRClientUpdateDlg();
// �Ի�������
	enum { IDD = IDD_MBRCLIENTUPDATE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
