// SysInfoDlg.cpp : implementation file
//
#include "stdafx.h"
#include "SysInfo.h"
#include "SysInfoDlg.h"
#include "SysInfoDefs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSysInfoDlg dialog

CSysInfoDlg::CSysInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSysInfoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSysInfoDlg)
	m_Response = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSysInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSysInfoDlg)
	DDX_Control(pDX, IDC_QUERY, m_Query);
	DDX_Control(pDX, IDC_COMPUTER, m_Computer);
	DDX_Text(pDX, IDC_RESPONSE, m_Response);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSysInfoDlg, CDialog)
	//{{AFX_MSG_MAP(CSysInfoDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_EDITCHANGE(IDC_QUERY, OnChangeQuery)
	ON_CBN_SELCHANGE(IDC_QUERY, OnSelChangeQuery)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSysInfoDlg message handlers

BOOL CSysInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}


	SetIcon(m_hIcon, TRUE);			//  big 
	SetIcon(m_hIcon, FALSE);		//  small 
	
	
	InitilizeCombo();
	m_Computer.Open(IDR_COMPUTER);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSysInfoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}


void CSysInfoDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSysInfoDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CSysInfoDlg::OnChangeQuery() 
{
	int QueryID=m_Query.GetItemData(m_Query.GetCurSel());
	m_Response=Process(QueryID);

	UpdateData(FALSE);
}

CString CSysInfoDlg::Process(int CommandID)
{
	CString Response="";

	switch (CommandID)
	{
		case Q_COMPUTER_NAME:
		{
			Response=QueryComputerName();
			break;
		}

		case Q_USER_NAME:
		{
			Response=QueryUserName();
			break;
		}

		case Q_IEXPLORER_VERSION:
		{
			Response=DetermineIEVer();
			break;
		}

		case Q_NUM_CPU:
		{
			Response=QueryCPUInfo();
			break;
		}


		case Q_CPU_SPEED:
		{
			Response=QueryCPUSpeed();
			break;
		}

		case Q_CPU_ID:
		{
			Response=QueryCPUID();
			break;				
		}

		case Q_CPU_VENDOR_ID:
		{
			Response=QueryCPUVendorID();
			break;				
		}

		case Q_RESOLUTION:
		{
			Response=QueryResolution();
			break;
		}

		case Q_COLOR_DEPTH:
		{
			Response=QueryPixelDepth();
			break;
		}
		
		case Q_RAM_TOTAL:
		{
			Response=QueryTotalRAM();
			break;
		}

		case Q_RAM_FREE:
		{
			Response=QueryFreeRAM();
			break;
		}

		case Q_HARD_NUMBER:
		{
			Response=QueryHardDisks();
			break;
		}

		/*case Q_PARTITIONS:
		{
			Response=QueryPartitions();
			break;
		}

		//case Q_HARD_SPACE:
		//case Q_HARD_FREE:
		case Q_HARD_USED:
		{
			Response=QueryDriveSpace();
			break;
		}*/

		case Q_HDD_SMART_COMMAND:
		{
			Response=QueryHDDSmartCommand();
			break;
		}

		case Q_CD_DRIVE:
		{
			Response=QueryCDDrive();
			break;
		}

		case Q_OS:
		{
			Response=QueryOS();
			break;
		}

		case Q_IP_ADDRESS:
		{
			Response=QueryLocalIPAddress();
			break;
		}

		case Q_OS_DIR:
		{
			Response=QueryDirectory("OS",CSIDL_WINDOWS);
			break;
		}

		case Q_SYSTEM_DIR:
		{
			Response=QueryDirectory("System",CSIDL_SYSTEM);
			break;
		}

		case Q_DESKTOP_DIR:
		{
			Response=QueryDirectory("Desktop",CSIDL_DESKTOPDIRECTORY);
			break;
		}

		case Q_COOKIES_DIR:
		{
			Response=QueryDirectory("Cookies",CSIDL_COOKIES);
			break;
		}

		case Q_STARTUP_DIR:
		{
			Response=QueryDirectory("Startup",CSIDL_STARTUP);
			break;
		}

		case Q_STARTMENU_DIR:
		{
			Response=QueryDirectory("Start Menu",CSIDL_STARTMENU);
			break;
		}

		case Q_FAVORITES_DIR:
		{
			Response=QueryDirectory("Favorites",CSIDL_FAVORITES);
			break;
		}

		case Q_FONTS_DIR:
		{
			Response=QueryDirectory("Fonts",CSIDL_FONTS);
			break;
		}

		case Q_INET_CACHE_DIR:
		{
			Response=QueryDirectory("Internet Cache",CSIDL_INTERNET_CACHE);
			break;
		}

		case Q_LOCAL_APP_DIR:
		{
			Response=QueryDirectory("Local Application",CSIDL_LOCAL_APPDATA);
			break;
		}

		case Q_MY_DOCUMENTS:
		{
			Response=QueryDirectory("My Documents",CSIDL_MYDOCUMENTS);
			break;
		}

		case Q_PROGRAM_FILES_DIR:
		{
			Response=QueryDirectory("Program Files",CSIDL_PROGRAM_FILES);
			break;
		}

		case Q_PROGRAM_GROUP_DIR:
		{
			Response=QueryDirectory("Program Group",CSIDL_PROGRAMS);
			break;
		}

		case Q_RECENT_DIR:
		{
			Response=QueryDirectory("Recent",CSIDL_RECENT);
			break;
		}

		case Q_SEND_TO_DIR:
		{
			Response=QueryDirectory("Send To",CSIDL_SENDTO);
			break;
		}

		case Q_TEMPLATES_DIR:
		{
			Response=QueryDirectory("Templates",CSIDL_TEMPLATES);
			break;
		}

		case Q_ADMIN_TOOLS_DIR:
		{
			Response=QueryDirectory("Administrative Tools",CSIDL_ADMINTOOLS);
			break;
		}

		case Q_HISTORY_DIR:
		{
			Response=QueryDirectory("History",CSIDL_HISTORY);
			break;
		}

		case Q_NET_HOOD_DIR:
		{
			Response=QueryDirectory("NetHood",CSIDL_NETHOOD);
			break;
		}

		case Q_PERSONAL_DIR:
		{
			Response=QueryDirectory("Personal",CSIDL_PERSONAL);
			break;
		}

		case Q_PROFILE_DIR:
		{
			Response=QueryDirectory("Profile",CSIDL_PROFILE);
			break;
		}

		case Q_COM_STARTUP_DIR:
		{
			Response=QueryDirectory("Common Startup",CSIDL_COMMON_STARTUP);
			break;
		}

		case Q_COM_PROG_DIR:
		{
			Response=QueryDirectory("Common Programs Group",CSIDL_COMMON_PROGRAMS);
			break;
		}

		case Q_COM_DESKTOP_DIR:
		{
			Response=QueryDirectory("Common Desktop",CSIDL_COMMON_DESKTOPDIRECTORY);
			break;
		}

		case Q_COM_FAVORITES_DIR:
		{
			Response=QueryDirectory("Common Favorites",CSIDL_COMMON_FAVORITES);
			break;
		}

		case Q_COM_STARTMENU_DIR:
		{
			Response=QueryDirectory("Common StartMenu",CSIDL_COMMON_STARTMENU);
			break;
		}

		case Q_COM_ADMIN_DIR:
		{
			Response=QueryDirectory("Common Administrative Tools",CSIDL_COMMON_ADMINTOOLS);
			break;
		}

		case Q_COM_APP_DATA_DIR:
		{
			Response=QueryDirectory("Common Application Data",CSIDL_COMMON_APPDATA);
			break;
		}

		case Q_COM_DOCUMENTS_DIR:
		{
			Response=QueryDirectory("Common Documents",CSIDL_COMMON_DOCUMENTS);
			break;
		}

		case Q_COM_TEMPLATES_DIR:
		{
			Response=QueryDirectory("Common Templates",CSIDL_COMMON_TEMPLATES);
			break;
		}

		default:
		{
			Response.Format("Невозможно определить запрос: %ld",CommandID);
			break;
		}
	}

	return Response;
}


void CSysInfoDlg::InitilizeCombo()
{
	for (int iCounter=0;iCounter<NUM_OF_QUERIES;iCounter++)
	{
		m_Query.AddString(Query[iCounter]);
		m_Query.SetItemData(iCounter,iCounter);
	}

	//Select first query
	m_Query.SetCurSel(0);	
}

void CSysInfoDlg::OnSelChangeQuery() 
{
	int QueryID=m_Query.GetItemData(m_Query.GetCurSel());
	m_Response=Process(QueryID);

	UpdateData(FALSE);
}
