// SysInfoDlg.h : header file
//

#ifndef _SYS_INFO_DLG_H_
#define _SYS_INFO_DLG_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CSysInfoDlg : public CDialog
{
// Construction
public:
	void InitilizeCombo();
	CSysInfoDlg(CWnd* pParent = NULL);	// standard constructor
	CString Process(int CommandID);

// Dialog Data
	//обмен данными
	//{{AFX_DATA(CSysInfoDlg)
	enum { IDD = IDD_SYSINFO_DIALOG };
	CComboBox	m_Query;
	CAnimateCtrl	m_Computer;
	CString	m_Response;
	//}}AFX_DATA

	
	//{{AFX_VIRTUAL(CSysInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// функции для создания сообщений
	//{{AFX_MSG(CSysInfoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnChangeQuery();
	afx_msg void OnSelChangeQuery();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(_SYS_INFO_DLG_H_)
