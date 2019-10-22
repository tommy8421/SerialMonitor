
// commonitorDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "commonitor.h"
#include "commonitorDlg.h"
#include "afxdialogex.h"
#include <stdio.h>
#include <Winsvc.h>
#include <wchar.h>

#define DRIVER_NAME "PdaAux"
#define DRIVER_PATH "PdaAux.sys"
#define SERVER_NAME "\\\\.\\PdaAux"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SERMON_READ 1
#define SERMON_WRITE 2
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CcommonitorDlg 对话框



CcommonitorDlg::CcommonitorDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_COMMONITOR_DIALOG, pParent)
	, m_port(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CcommonitorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_COM, m_port);
	//DDX_Control(pDX, IDC_EDIT_RESULT2, m_edit1);
	DDX_Control(pDX, IDC_EDIT_RESULT, m_edit2);
}

BEGIN_MESSAGE_MAP(CcommonitorDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CcommonitorDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_START, &CcommonitorDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_LOAD, &CcommonitorDlg::OnBnClickedButtonLoad)
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CcommonitorDlg::OnBnClickedButtonTest)
	ON_BN_CLICKED(IDC_BUTTON_UNLOAD, &CcommonitorDlg::OnBnClickedButtonUnload)
END_MESSAGE_MAP()


// CcommonitorDlg 消息处理程序
HANDLE hDevice = INVALID_HANDLE_VALUE;

bool OpenedPorts[255];
BOOL CcommonitorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CcommonitorDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CcommonitorDlg::OnPaint()
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
HCURSOR CcommonitorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CcommonitorDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	if (pThread != NULL)
	{
		SetEvent(hEvent);
		WaitForSingleObject(pThread->m_hThread, INFINITE);
		CloseHandle(hEvent);
		delete pThread;
	//	CloseHandle(pThread);
		//WaitForsingleObject(pThread->m_hThread, INFINITE);
		//delete pThread;
	}
	// TODO: 在此处添加消息处理程序代码
	if (hDevice != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hDevice);
		SC_HANDLE sc = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE,
			SC_MANAGER_ALL_ACCESS);
		if (sc)
		{
			SC_HANDLE hSrv = OpenService(sc, _T(DRIVER_NAME), SERVICE_START | SERVICE_STOP | DELETE);
			CloseServiceHandle(sc);
			if (hSrv)
			{
				SERVICE_STATUS ss;
				ControlService(hSrv, SERVICE_CONTROL_STOP, &ss);
				CloseServiceHandle(hSrv);
			}
		}
	}
}


void CcommonitorDlg::OnBnClickedButtonStop()
{
	// TODO: 在此添加控件通知处理程序代码
	DWORD dw;
	BOOL res = DeviceIoControl(hDevice, IOCTL_SERMON_STOPMONITOR, &handle, sizeof(MHANDLE), NULL, NULL,
		&dw, NULL);
	if (!res)
	{
		AfxMessageBox(_T("Unexpected error occured"), MB_OK);
	}
	else
	{
	//	handle = NULL;
		memset(&handle, 0x00, sizeof(handle));
		OpenedPorts[m_Port] = false;
	}
}
BOOL CcommonitorDlg::setPort(int port) {
	// TODO: 在此添加控件通知处理程序代码
	m_Port = port;

	
	if (hDevice != INVALID_HANDLE_VALUE) {
		return TRUE;
	}
	// 打开设备
	hDevice = CreateFile(SERVER_NAME,
		GENERIC_READ | GENERIC_WRITE |GENERIC_EXECUTE, 
		FILE_SHARE_WRITE | FILE_SHARE_READ,
		NULL, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL |	FILE_FLAG_OVERLAPPED,
		NULL);

	if (hDevice == INVALID_HANDLE_VALUE)
	{
		CString s;
		s.Format("请检查设备是否存在:[%d]", GetLastError());
		MessageBox(s);
		return FALSE;
	}
	//CloseHandle(hDevice);
	
	if (OpenedPorts[m_Port])
		return FALSE;

	DWORD dw;
	CString s;

	/*
		打开设备
	*/
	//handle = 
	//s.Format("\\??\\COM%d", m_Port);
	//m_nSelPort = m_ctlPort.GetCurSelComNo();
	BYTE	szComName[40 + 1];
	WCHAR	szWComName[40 + 1];

	int nLength = sprintf((char *)szComName, _T("\\??\\COM%d"), m_Port) + 1;
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)szComName, nLength, szWComName, 41*sizeof(WCHAR));

	INIT_MHANDLE(handle);
	BOOL res = DeviceIoControl(hDevice, 
		IOCTL_SERMON_STARTMONITOR, 
	//	(PVOID)(LPCTSTR)s,
	//	(s.GetLength() + 1) * sizeof(WCHAR),
		szWComName,
		nLength * sizeof(WCHAR),
		&handle, 
		sizeof(MHANDLE), 
		&dw, 
		NULL);
	if (!res)
	{
		CString s;
		s.Format(_T("Cannot start monitor for %d communication port. Port maybe already opened[%d]"), m_Port, GetLastError());
		AfxMessageBox(s, MB_OK);
		return FALSE;
	}

	OpenedPorts[m_Port] = true;
	return TRUE;
}

void CcommonitorDlg::OnBnClickedButtonStart()
{
	UpdateData(TRUE);
	BOOL bl = setPort(atol((char *)m_port.GetString()));
	if (!bl) {
		MessageBox("设置端口错误");
		return;
	}
	printf("handle0:[%02x %02x]", handle.nData);
	hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	ASSERT(hEvent);
	pThread = AfxBeginThread((AFX_THREADPROC)_Thread, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	ASSERT(pThread);
	pThread->m_bAutoDelete = FALSE;
	pThread->ResumeThread();

}
UINT CcommonitorDlg::_Thread(CcommonitorDlg *pView) {
	return pView->Thread();
}
UINT CcommonitorDlg::Thread(void)
{
	// Issue GetInfoSize request
	DWORD Size, dw;
	OVERLAPPED over;

	if (hDevice == INVALID_HANDLE_VALUE) {
		return 0;
	}
	printf("handle:[%02x %02x]", handle.nData);
	ZeroMemory(&over, sizeof(OVERFLOW));
	over.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	HANDLE hEvents[] = { over.hEvent,hEvent };
	while (1)
	{
		BOOL res = DeviceIoControl(hDevice, 
			IOCTL_SERMON_GETINFOSIZE, 
			&handle, 
			sizeof(MHANDLE),
			&Size, 
			sizeof(DWORD),
			&dw, 
			&over);
		if (res == TRUE)	// request completed
		{
		//	MessageBox("成功");
			if (dw == sizeof(DWORD) && Size)
				goto GetInfo;
		}
		else
		{
			CString s;
		//	s.Format(_T("error:%d"), GetLastError());
	//		MessageBox(s);
			if (GetLastError() != ERROR_IO_PENDING)
			{
				MessageBox("失败");
				CancelIo(hDevice);
				return 0;
			}
			DWORD Res = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
			BOOL flag = GetOverlappedResult(hDevice, &over, &dw, FALSE);
			if (Res == WAIT_OBJECT_0 + 1 || !flag)	// close
			{
				CancelIo(hDevice);
				return 0;
			}
		}
	GetInfo:
		LPBYTE p = new BYTE[Size];
		res = DeviceIoControl(hDevice,
			IOCTL_SERMON_GETINFO, 
			&handle, 
			sizeof(MHANDLE),
			p, 
			Size, 
			&dw, 
			&over);
		if (res == TRUE)	// request completed
		{
		//	processIOReq((IOReq *)p);
			ProcessIOReq(p, dw);
			continue;
		}
		if (GetLastError() != ERROR_IO_PENDING)
		{
			delete[]p;
			return 0;
		}
		DWORD Res = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
		BOOL flag = GetOverlappedResult(hDevice, &over, &dw, FALSE);
		if (Res == WAIT_OBJECT_0 + 1 || !flag)	// close
		{
			//			CancelIo(hDevice);
			delete[]p;
			return 0;
		}
		//processIOReq((IOReq *)p);
		ProcessIOReq(p, dw);
	}
}
/*
void CcommonitorDlg::PrintBoth(LPCTSTR String)
{

	int last;
	int to;
	int from;

	last = m_edit2.GetWindowTextLength();
	m_edit2.SetSel(last, last);
	m_edit2.ReplaceSel(_T("\n"));
	Char2 = 0;
	Line2++;	// next line
	to = m_edit2.LineIndex(Line2 + 1);
	if (to == -1)
		to = m_edit2.GetWindowTextLength();
	from = m_edit2.LineIndex(Line2);
	m_edit2.SetSel(from, to);
	m_edit2.ReplaceSel(String);
	Line2++;
}
*/
/*
void CcommonitorDlg::processIOReq(IOReq *p) {
	int j;
	Char1 = 0;
	Char2 = 0;
	union
	{
		WCHAR chr;
		BYTE value;
	} temp;
	temp.chr = 0;
	switch (p->m_nType)
	{
	case REQ_OPEN:
	case REQ_CLOSE:	// these two requests will go in both windows
	{
		if (p->m_nType == REQ_OPEN)
			PrintBoth(_T("Port opened\n"));
		else
			PrintBoth(_T("Port closed\n"));


		break;
	}
	case REQ_READ:
	{
		LPBYTE ptr = (LPBYTE)p + sizeof(IOReq);
		DWORD Size = p->m_nSizeCopied;
		for (DWORD i = 0; i < Size; i++)
		{
			if (Char1 <= 15)
				Symbols1[Char1++] = *ptr++;
			else
			{
				// replace required string with new one
				CString s, t;
				for (int j = 0; j < 16; j++)
				{
					t.Format(_T("%02X "), Symbols1[j]);
					s += t;
				}
				for (j = 0; j < 16; j++)
				{
					temp.value = (Symbols1[j] >= 32) ? Symbols1[j] : '.';
					t.Format(_T("%c"), temp.chr);
					s += t;
				}
				s += "\n";
				int from = m_edit2.LineIndex(Line2);
				int to = m_edit2.LineIndex(Line2 + 1);
				if (to == -1)
					to = m_edit2.GetWindowTextLength();
				m_edit2.SetSel(from, to, TRUE);
				m_edit2.ReplaceSel(s);
				Line2++;
				Char1 = 1;
				Symbols1[0] = *ptr++;
			}
		}
		// replace required string with new one
		if (Char1)
		{
			CString s, t;
			for (j = 0; j < 16; j++)
			{
				if (j < Char1)
					t.Format(_T("%02X "), Symbols1[j]);
				else
					t = _T("\t");
				s += t;
			}
			for (j = 0; j < Char1; j++)
			{
				temp.value = (Symbols1[j] >= 32) ? Symbols1[j] : '.';
				t.Format(_T("%c"), temp.chr);
				s += t;
			}
			s += "\n";
			int from = m_edit2.LineIndex(Line2);
			int to = m_edit2.LineIndex(Line2+ 1);
			if (to == -1)
				to = m_edit2.GetWindowTextLength();
			m_edit2.SetSel(from, to);
			m_edit2.ReplaceSel(s);
		}
		break;
	}
	case REQ_WRITE:
	{
		LPBYTE ptr = (LPBYTE)p + sizeof(IOReq);
		DWORD Size = p->m_nSizeCopied;
		for (DWORD i = 0; i < Size; i++)
		{
			if (Char2 < 15)
				Symbols2[Char2++] = *ptr++;
			else
			{
				// replace required string with new one
				CString s, t;
				for (j = 0; j < 16; j++)
				{
					t.Format(_T("%02X "), Symbols2[j]);
					s += t;
				}
				for (j = 0; j < 16; j++)
				{
					temp.value = (Symbols2[j] >= 32) ? Symbols2[j] : '.';
					t.Format(_T("%c"), temp.chr);
					s += t;
				}
				s += _T("\n");
				int from = m_edit2.LineIndex(Line2);
				int to = m_edit2.LineIndex(Line2 + 1);
				if (to == -1)
					to = m_edit2.GetWindowTextLength();
				m_edit2.SetSel(from, to, TRUE);
				m_edit2.ReplaceSel(s);
				Line2++;
				Char2 = 1;
				Symbols2[0] = *ptr++;
			}
		}
		// replace required string with new one
		if (Char2)
		{
			
			CString s, t;
			for (j = 0; j < 16; j++)
			{
				if (j < Char2)
					t.Format(_T("%02X "), Symbols2[j]);
				else
					t = _T("\t");
				s += t;
			}
			for (j = 0; j < Char2; j++)
			{
				temp.value = (Symbols2[j] >= 32) ? Symbols2[j] : '.';
				t.Format(_T("%c"), temp.chr);
				s += t;
			}
			s += _T("\n");
			int from = m_edit2.LineIndex(Line2);
			int to = m_edit2.LineIndex(Line2 + 1);
			if (to == -1)
				to = m_edit2.GetWindowTextLength();
			m_edit2.SetSel(from, to);
			m_edit2.ReplaceSel(s);
		}
		break;
	}
	case REQ_SET_BAUD_RATE:
	{
		CString s;
		s.Format(_T("Baud rate set to %d\n"),
			*(ULONG *)((LPBYTE)p + sizeof(IOReq)));
		PrintBoth(s);
		break;
	}
	case REQ_SET_LINE_CONTROL:
	{
		CString s;
		LPCTSTR temp;
		SERIAL_LINE_CONTROL *ptr = (SERIAL_LINE_CONTROL *)((LPBYTE)p + sizeof(IOReq));

		switch (ptr->Parity)
		{
		case EVENPARITY:
			temp = _T("Even");
			break;
		case MARKPARITY:
			temp = _T("Mark");
			break;
		case NOPARITY:
			temp = _T("No");
			break;
		case ODDPARITY:
			temp = _T("Odd");
			break;
		case SPACEPARITY:
			temp = _T("Space");
			break;
		default:
			temp = _T("Strange");
			break;
		}

		s.Format(_T("StopBits: %s, Parity: %s parity, WordLength: %d\n"),
			((ptr->StopBits == ONESTOPBIT) ? _T("1 stop bit") : ((ptr->StopBits == ONE5STOPBITS) ?
				_T("1.5 stop bits") : _T("2 stop bits"))), temp,
			ptr->WordLength);
		PrintBoth(s);
		break;
	}
	}
	delete[](LPBYTE) p;
}

BOOL CcommonitorDlg::NeedProcessIOReq(ULONG nReqType)
{
	return (CSetSermonOpt::GetOptionByReqType(nReqType) & m_dwOptions);
}
*/
void CcommonitorDlg::ProcessIOReq(PBYTE pMonInfo, DWORD dwDataLen)
{
	PBYTE	pExtInfo = pMonInfo + sizeof(IOReq);
	IOReq	*pIOReq = (IOReq *)pMonInfo;
	CString s;

	if (dwDataLen < sizeof(IOReq))
	{
		s.Format(_T("\r\nData size(=%lu) is too small, skipping..."), dwDataLen);
		AddNormalString(s);
		return;
	}

	TRACE1(_T("SerMon IOReq type:%d\r\n"), pIOReq->m_nType);
	switch (pIOReq->m_nType)
	{
	case REQ_OPEN:
	case REQ_CLOSE:
		ProcessOpenClose(pIOReq->m_nType);
		break;

	case REQ_READ:
	case REQ_WRITE:
//		ProcessReadWrite(pIOReq->m_nType == REQ_READ ? SERMON_READ : SERMON_WRITE, pExtInfo, pIOReq->m_nSizeCopied);
		ProcessReadWrite(pIOReq->m_nType, pExtInfo, pIOReq->m_nSizeCopied);
		break;

	case REQ_SET_BAUD_RATE:
		ProcessSetBaud(((PSERIAL_BAUD_RATE)pExtInfo)->BaudRate);
		break;

	case REQ_SET_LINE_CONTROL:
		ProcessSetLineCtrl((PSERIAL_LINE_CONTROL)pExtInfo);
		break;

	case REQ_SET_QUEUE_SIZE:
		ProcessSetQueueSize((PSERIAL_QUEUE_SIZE)pExtInfo);
		break;

	case REQ_SET_TIMEOUTS:
		ProcessSetTimeouts((PSERIAL_TIMEOUTS)pExtInfo);
		break;

	case REQ_SERIAL_PURGE:
		ProcessSerialPurge(*((ULONG *)pExtInfo));
		break;

	case REQ_SET_RTS:
	case REQ_CLR_RTS:
	case REQ_SET_DTR:
	case REQ_CLR_DTR:
	case REQ_RESET_DEVICE:
		ProcessNoDataCtrl(pIOReq->m_nType);
		break;

	default:
		CString s;
		s.Format(_T("\r\nUnknown IO type(%lu), skipping..."), pIOReq->m_nType);
		AddNormalString(s);
		break;
	}
}

void CcommonitorDlg::ProcessOpenClose(int nType)
{
	CString s;
	if (nType == REQ_OPEN)
	{
		s.Format(_T("\r\n------------------------------------>\r\nCOM%d is opened."), m_Port);
		AddNormalString(s);
	}
	else
	{
		s.Format(_T("\r\nCOM%d is closed.\r\n<------------------------------------"), m_Port);
		AddNormalString(s);
	}
}

void CcommonitorDlg::ProcessSetBaud(ULONG nBaudRate)
{
	CString s;
	s.Format(_T("\r\nCOM%d Set Baud Rate: %lu."), m_Port, nBaudRate);
	AddNormalString(s);
}
void CcommonitorDlg::ProcessSetLineCtrl(SERIAL_LINE_CONTROL *pCtrl)
{
	CString		strParity;

	switch (pCtrl->Parity)
	{
	case EVENPARITY:
		strParity = _T("Even");
		break;

	case MARKPARITY:
		strParity = _T("Mark");
		break;

	case NOPARITY:
		strParity = _T("No");
		break;

	case ODDPARITY:
		strParity = _T("Odd");
		break;

	case SPACEPARITY:
		strParity = _T("Space");
		break;

	default:
		strParity = _T("Strange");
		break;
	}
	CString s;
	s.Format(_T("\r\nStopBits: %s, Parity: %s, DataBits: %d."),
		((pCtrl->StopBits == ONESTOPBIT) ? _T("1") : ((pCtrl->StopBits == ONE5STOPBITS) ? _T("1.5") : _T("2"))),
		(LPCSTR)strParity,
		pCtrl->WordLength);
	AddNormalString(s);
}

void CcommonitorDlg::ProcessReadWrite(int nSerMonOper, PBYTE psData, int nDataLen)
{
	int i;
	CString s;
	//BYTE temp;
	unsigned char temp[10];
	if (nDataLen <= 0)
	{
		return;
	}
	memset(temp, 0x00, sizeof(temp));
	if (nSerMonOper == REQ_READ)
	{
		AddNormalString(_T("\r\nRead bytes:\r\n"));
	}
	else
	{
		AddNormalString(_T("\r\nWrite bytes:\r\n"));
	}

	// 16进制显示
	for (i = 0; i < nDataLen; i++)
	{
		
		sprintf((char *)temp, "%02x ", psData[i]);
		s.Append((LPCSTR)temp);
		if ((i + 1) % 16 == 0)
		{
			s.Append("\r\n");
		}
	}
	AddNormalString(s);
}

void CcommonitorDlg::ProcessNoDataCtrl(int nType)
{
	static struct
	{
		int		nReqType;
		LPCSTR	lpszMessage;
	}
	arInfoList[] =
	{
		{REQ_SET_RTS,		_T("Set RTS")},
		{REQ_CLR_RTS,		_T("Clear RTS")},
		{REQ_SET_DTR,		_T("Set DTR")},
		{REQ_CLR_DTR,		_T("Clear DTR")},
		{REQ_RESET_DEVICE,	_T("Reset Device")},
	};
	CString s;

	for (int nCnt = 0; nCnt < 5; nCnt++)
	{
		if (nType == arInfoList[nCnt].nReqType)
		{
			s.Format(_T("\r\nCOM%d %s."), m_Port, arInfoList[nCnt].lpszMessage);
			AddNormalString(s);
			break;
		}
	}
}

void CcommonitorDlg::ProcessSetQueueSize(PSERIAL_QUEUE_SIZE pQueueSize)
{
	CString s;
	s.Format(_T("\r\nCOM%d Set Queue Size, In=%lu, Out=%lu."), m_Port, pQueueSize->InSize, pQueueSize->OutSize);
	AddNormalString(s);
}

void CcommonitorDlg::ProcessSetTimeouts(PSERIAL_TIMEOUTS pTimeOuts)
{
	CString s;
	s.Format(_T("\r\nCOM%d Set ReadIntervalTimeout=%lu(0x%X), ReadTotalTimeoutConstant=%lu(0x%X), ReadTotalTimeoutMultiplier=%lu(0x%X), WriteTotalTimeoutConstant=%lu(0x%X), WriteTotalTimeoutMultiplier=%lu(0x%X)."),
		m_Port,
		pTimeOuts->ReadIntervalTimeout, pTimeOuts->ReadIntervalTimeout,
		pTimeOuts->ReadTotalTimeoutConstant, pTimeOuts->ReadTotalTimeoutConstant,
		pTimeOuts->ReadTotalTimeoutMultiplier, pTimeOuts->ReadTotalTimeoutMultiplier,
		pTimeOuts->WriteTotalTimeoutConstant, pTimeOuts->WriteTotalTimeoutConstant,
		pTimeOuts->WriteTotalTimeoutMultiplier, pTimeOuts->WriteTotalTimeoutMultiplier);
	AddNormalString(s);
}

void CcommonitorDlg::ProcessSerialPurge(ULONG ulPurgeInfo)
{
	CString		strInfo;

	if (ulPurgeInfo & SERIAL_PURGE_TXABORT)
	{
		strInfo += _T("PURGE_TXABORT,");
	}

	if (ulPurgeInfo & SERIAL_PURGE_RXABORT)
	{
		strInfo += _T("PURGE_RXABORT,");
	}

	if (ulPurgeInfo & SERIAL_PURGE_TXCLEAR)
	{
		strInfo += _T("PURGE_TXCLEAR,");
	}

	if (ulPurgeInfo & SERIAL_PURGE_RXCLEAR)
	{
		strInfo += _T("PURGE_RXCLEAR,");
	}

	strInfo.TrimRight(_T(","));
	if (!strInfo.IsEmpty())
	{
		CString s;
		s.Format(_T("\r\nCOM%d Operation: %s."), m_Port, (LPCTSTR)strInfo);
		AddNormalString(s);
	}
}

void CcommonitorDlg::AddNormalString(CString fmt)
{
	int len = m_edit2.GetWindowTextLength();
	m_edit2.SetSel(len, len); // 将插入光标放在最后 
	m_edit2.ReplaceSel(fmt);
	m_edit2.ScrollWindow(0, 0); // 滚动到插入点 (滚动条始终在底部，不闪动)
	if (m_edit2.GetWindowTextLength() > (65535 / 2)) // 28*1000 经验值
	{
		m_edit2.SetWindowText(_T(""));
	}
}
//装载NT驱动程序
BOOL CcommonitorDlg::LoadNTDriver(char* lpszDriverName, char* lpszDriverPath)
{

	/************************ 加载NT驱动的代码*******************************
	   ① 调用OpenSCManager,打开SCM管理器.如果返回NULL,则返回失败,否则继续
	   ② 调用CreateService,创建服务,创建成功则转步骤 ⑥
		  ③ 用GetLastError的得到错误返回值
	   ④ 返回值为ERROR_IO_PENDING,说明服务已经创建过,用OpenService打开此服务.
	   ⑤ 返回值为其他值, 创建武服务失败,返回失败.
	   ⑥ 调用StartService开启服务
	   ⑦ 成功返回
	************************************************************************/

	char szDriverImagePath[256];


	BOOL bRet = FALSE;

	SC_HANDLE hServiceMgr = NULL;// SCM管理器的句柄
	SC_HANDLE hServiceDDK = NULL;// NT驱动程序的服务句柄

	memset(szDriverImagePath, 0x00, sizeof(szDriverImagePath));

	//得到完整的驱动路径
	int retval = GetFullPathName((LPCSTR)lpszDriverPath, 256, (LPSTR)szDriverImagePath, NULL);

	if (retval == 0)
	{
		// Handle an error condition.
		CString s;
		s.Format("GetFullPathName failed (%d)\n", GetLastError());
		MessageBox(s);
		return FALSE;
	}
	else
	{
		CString s;
		s.Format("fullname:%s", szDriverImagePath);
		MessageBox(s);
		//MessageBox(("The full path name is:  %s\n"), buffer);
		//if (lppPart != NULL && *lppPart != 0)
		//{
		//	_tprintf(TEXT("The final component in the path name is:  %s\n"), *lppPart);
		//}
	}
	//打开服务控制管理器
	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hServiceMgr == NULL)
	{
		// OpenSCManager失败
		printf("OpenSCManager() Faild %d ! \n", GetLastError());
		bRet = FALSE;
		goto BeforeLeave;
	}
	else
	{
		// OpenSCManager成功
		printf("OpenSCManager() ok ! \n");
	}


	//创建驱动所对应的服务
	hServiceDDK = CreateService(hServiceMgr,
		(LPCSTR)lpszDriverName,         // 驱动程序的在注册表中的名字 
		(LPCSTR)lpszDriverName,         // 注册表驱动程序的 DisplayName 值 
		SERVICE_ALL_ACCESS,     // 加载驱动程序的访问权限 
		SERVICE_KERNEL_DRIVER, // 表示加载的服务是驱动程序 
		SERVICE_DEMAND_START,   // 注册表驱动程序的 Start 值 
		SERVICE_ERROR_IGNORE,   // 注册表驱动程序的 ErrorControl 值 
		(LPCSTR)szDriverImagePath,      // 注册表驱动程序的 ImagePath 值 
		NULL,
		NULL,
		NULL,
		NULL,
		NULL);

	DWORD dwRtn;
	// 判断服务是否失败
	if (hServiceDDK == NULL)
	{
		dwRtn = GetLastError();
		if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS)
		{
			//由于其他原因创建服务失败
			CString s;
			s.Format("CrateService() Faild %d ! \n", dwRtn);
			MessageBox(s);
			bRet = FALSE;
			goto BeforeLeave;
		}
		else
		{
			//服务创建失败，是由于服务已经创立过
			MessageBox("CrateService() Faild Service is ERROR_IO_PENDING or ERROR_SERVICE_EXISTS! \n");
		}

		// 驱动程序已经加载，只需要打开 
		hServiceDDK = OpenService(hServiceMgr, (LPCSTR)lpszDriverName, SERVICE_ALL_ACCESS);
		if (hServiceDDK == NULL)
		{
			// 如果打开服务也失败，则意味错误
			dwRtn = GetLastError();
			CString s;
			s.Format("OpenService() Faild %d ! \n", dwRtn);
			MessageBox(s);
			bRet = FALSE;
			goto BeforeLeave;
		}
		else
		{
			MessageBox("OpenService() ok ! \n");
		}
	}
	else
	{
		MessageBox("CrateService() ok ! \n");
	}

	// 开启此项服务
	bRet = StartService(hServiceDDK, NULL, NULL);
	if (!bRet)
	{
		DWORD dwRtn = GetLastError();
		if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_ALREADY_RUNNING)
		{
			CString s;

			s.Format("StartService() Faild %d ! \n", dwRtn);
			MessageBox(s);
			bRet = FALSE;
			goto BeforeLeave;
		}
		else
		{
			if (dwRtn == ERROR_IO_PENDING)
			{
				// 设备被挂住
				MessageBox("StartService() Faild ERROR_IO_PENDING ! \n");
				bRet = FALSE;
				goto BeforeLeave;
			}
			else
			{
				// 服务已经开启
				MessageBox("StartService() Faild ERROR_SERVICE_ALREADY_RUNNING ! \n");
				bRet = TRUE;
				goto BeforeLeave;
			}
		}
	}
	bRet = TRUE;
	// 离开前关闭句柄
BeforeLeave:
	if (hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK); // 服务句柄
	}
	if (hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr); // SCM句柄
	}
	return bRet;
}

// 卸载驱动程序 
BOOL CcommonitorDlg::UnloadNTDriver(char * szSvrName)
{
	/************************* 卸载NT驱动的代码******************************
	   ① 调用OpenSCManager,打开SCM管理器,如果返回NULL,则返回失败,否则继续.
	   ② 调用OpenService.如果返回NULL,则返回失败,否则继续
	   ③ 调用DeleteService卸载此项服务.
	   ④ 成功返回.
	************************************************************************/

	BOOL bRet = FALSE;
	SC_HANDLE hServiceMgr = NULL;// SCM管理器的句柄
	SC_HANDLE hServiceDDK = NULL;// NT驱动程序的服务句柄
	SERVICE_STATUS SvrSta;
	// 打开SCM管理器
	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hServiceMgr == NULL)
	{
		// 打开SCM管理器失败
		CString s;
		s.Format("OpenSCManager() Faild %d ! \n", GetLastError());
		MessageBox(NULL,s);
		bRet = FALSE;
		goto BeforeLeave;
	}
	else
	{
		// 打开SCM管理器失败成功
		MessageBox("OpenSCManager() ok ! \n");
	}

	// 打开驱动所对应的服务
	hServiceDDK = OpenService(hServiceMgr, (LPCSTR)szSvrName, SERVICE_ALL_ACCESS);

	if (hServiceDDK == NULL)
	{
		// 打开驱动所对应的服务失败
		CString s;
		s.Format("OpenService() Faild %d ! \n", GetLastError());
		MessageBox(s);
		bRet = FALSE;
		goto BeforeLeave;
	}
	else
	{
		MessageBox("OpenService() ok ! \n");
	}

	// 停止驱动程序，如果停止失败，只有重新启动才能，再动态加载。 
	if (!ControlService(hServiceDDK, SERVICE_CONTROL_STOP, &SvrSta))
	{
		CString s;
		s.Format("ControlService() Faild %d !\n", GetLastError());
		MessageBox(s);
	}
	else
	{
		// 打开驱动所对应的失败
		MessageBox("ControlService() ok !\n");
	}
	// 动态卸载驱动程序。 
	if (!DeleteService(hServiceDDK))
	{
		// 卸载失败
		CString s;
		s.Format("DeleteSrevice() Faild %d !\n", GetLastError());
		MessageBox(s);
	}
	else
	{
		// 卸载成功
		MessageBox("DelServer:eleteSrevice() ok !\n");
	}
	bRet = TRUE;
BeforeLeave:
	// 离开前关闭打开的句柄
	if (hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK); // 服务句柄
	}
	if (hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr); // SCM 句柄
	}
	return bRet;
}

void CcommonitorDlg::TestDriver()
{
	// 测试驱动程序 
	HANDLE hDevice = CreateFile((LPCSTR)SERVER_NAME,
		GENERIC_WRITE | GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if (hDevice != INVALID_HANDLE_VALUE)
	{
		MessageBox("Create Device ok ! \n SUCESSFULLY....ComeOn...");

	}
	else
	{
		CString s;
		s.Format("Create Device faild %d ! \n", GetLastError());
		MessageBox(s);
	}
	CloseHandle(hDevice);
}


void CcommonitorDlg::OnBnClickedButtonLoad()
{
	// TODO: 在此添加控件通知处理程序代码
		// 加载驱动
		BOOL bRet = LoadNTDriver((char *)DRIVER_NAME, (char *)DRIVER_PATH);
		if (!bRet)
		{
			MessageBox("LoadNTDriver error\n");
			
		}
		else
		{
			MessageBox("LoadNTDriver successful");
		}
}


void CcommonitorDlg::OnBnClickedButtonTest()
{
	// TODO: 在此添加控件通知处理程序代码
	// 加载成功
	TestDriver();
}


void CcommonitorDlg::OnBnClickedButtonUnload()
{
	// TODO: 在此添加控件通知处理程序代码
	// 这时候你可以通过注册表，或其他查看符号连接的软件验证。 
	// 卸载驱动
	UnloadNTDriver((char *)DRIVER_NAME);
}
