
// commonitorDlg.h: 头文件
//

#pragma once
#include "SerMonEx.h"

// CcommonitorDlg 对话框
class CcommonitorDlg : public CDialogEx
{
// 构造
public:
	CcommonitorDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_COMMONITOR_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	HANDLE hEvent;
	CWinThread *pThread;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonStart();
	BOOL setPort(int port);
//	void processIOReq(IOReq *p);
	void ProcessIOReq(PBYTE pMonInfo, DWORD dwDataLen);
	void PrintBoth(LPCTSTR String);
	static UINT _Thread(CcommonitorDlg *pView);
	UINT Thread(void);
	BOOL LoadNTDriver(char* lpszDriverName, char* lpszDriverPath);
	BOOL UnloadNTDriver(char * szSvrName);
	void AddNormalString(CString fmt);
	void ProcessSerialPurge(ULONG ulPurgeInfo);
	void ProcessSetTimeouts(PSERIAL_TIMEOUTS pTimeOuts);
	void ProcessSetQueueSize(PSERIAL_QUEUE_SIZE pQueueSize);
	void ProcessNoDataCtrl(int nType);
	void ProcessReadWrite(int nSerMonOper, PBYTE psData, int nDataLen);
	void ProcessSetLineCtrl(SERIAL_LINE_CONTROL *pCtrl);
	void ProcessSetBaud(ULONG nBaudRate);
	void ProcessOpenClose(int nType);
	void TestDriver();
	CString m_port;
	int m_Port;
	MHANDLE handle;
	//CEdit m_edit1;
	CEdit m_edit2;
	int Line1, Line2, Char1, Char2;
	BYTE Symbols1[16], Symbols2[16];
	afx_msg void OnBnClickedButtonLoad();
	afx_msg void OnBnClickedButtonTest();
	afx_msg void OnBnClickedButtonUnload();
};
