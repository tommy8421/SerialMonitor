
#ifndef _DEVEXT_H_
#define _DEVEXT_H_

#define SERMON_DRIVER

#include "SerMonEx.h"

class CAttachedDevice;

extern PDRIVER_OBJECT					g_pDriverObj;
extern CDBLinkedList<CAttachedDevice>	*g_pDevList;

typedef struct _tagExIRP
{
	PIRP		m_pIRP;
	LIST_ENTRY	m_Entry;
}ExIRP, *PExIRP;

class CDevice
{
public:
	CDevice()
	{
	}

	virtual ~CDevice()
	{
	}

protected:
	NTSTATUS DoDefault(PIRP pIRP)
	{
		IoCompleteRequest(pIRP, IO_NO_INCREMENT);

		pIRP->IoStatus.Information = 0;
		pIRP->IoStatus.Status      = STATUS_SUCCESS;

		return STATUS_IO_DEVICE_ERROR;
	}
	
public:
	virtual NTSTATUS IoControl(PIRP pIRP)
	{
		return DoDefault(pIRP);
	}

	virtual NTSTATUS Read(PIRP pIRP)
	{
		return DoDefault(pIRP);
	}
	
	virtual NTSTATUS Write(PIRP pIRP)
	{
		return DoDefault(pIRP);
	}
	
	virtual NTSTATUS Open(PIRP pIRP)
	{
		return DoDefault(pIRP);
	}
	
	virtual NTSTATUS Close(PIRP pIRP)
	{
		return DoDefault(pIRP);
	}
	
	virtual NTSTATUS Cleanup(PIRP pIRP)
	{
		return DoDefault(pIRP);
	}
	
	virtual NTSTATUS Flush(PIRP pIRP)
	{
		return DoDefault(pIRP);
	}
};

class CSerMonDevice: public CDevice
{
public:
	CSerMonDevice();
	~CSerMonDevice();

protected:
	NTSTATUS Attach(PUNICODE_STRING puszDevName, DWORD32 dwDesiredAccess, PDEVICE_OBJECT *pRetDevObj);
	BOOL ConnectToSerialDevice(LPCTSTR lpszBufferName, PMHANDLE pMHandle);
	CAttachedDevice *GetAttachedDevice(PVOID pIrpBuffer);
	BOOL IsValidDevice(CAttachedDevice *pDevice);

public:
	virtual NTSTATUS IoControl(PIRP pIRP);
};

NTSTATUS DefaultCompletion(IN PDEVICE_OBJECT pDevObj, IN PIRP pIRP, IN PVOID pContext);
NTSTATUS ReadCompletion(IN PDEVICE_OBJECT  pDevObj, IN PIRP pIRP, IN PVOID pContext);
NTSTATUS WriteCompletion(IN PDEVICE_OBJECT pDevObj, IN PIRP pIRP, IN PVOID pContext);
NTSTATUS CloseCompletion(IN PDEVICE_OBJECT pDevObj, IN PIRP pIRP, IN PVOID pContext);
NTSTATUS OpenCompletion(IN PDEVICE_OBJECT  pDevObj, IN PIRP pIRP, IN PVOID pContext);
NTSTATUS IoControlCompletion(IN PDEVICE_OBJECT pDevObj, IN PIRP pIRP, IN PVOID pContext);

class CAttachedDevice : public CDevice
{
public:
	CAttachedDevice();
	~CAttachedDevice();

protected:
	WCHAR					m_Signature[3];
	ERESOURCE				m_stERES;
	CDBLinkedList<IOReq>	m_listIoreq;		// request queue
	CDBLinkedList<ExIRP>	m_listPending;		// m_kEvent queue

public:
	KEVENT					m_kEvent;
	LONG					m_nNum;
	BOOL					m_bFirstTime;
	PDEVICE_OBJECT			m_pOrgDevice;
	PDEVICE_OBJECT			m_pThisDevice;		// device represented by this object
	LIST_ENTRY				m_Entry;

protected:
	NTSTATUS Standard(PIRP Irp, PIO_COMPLETION_ROUTINE Routine=NULL);
	NTSTATUS ProcessSize(PIRP pIRP, IOReq *pIOReq);
	NTSTATUS ProcessNext(PIRP pIRP, IOReq *pIOReq);
	void ProcessEmpty(PIRP pIRP);

public:
	BOOL CheckValid();
	void New(IOReq *pIOReq);
	NTSTATUS GetNext(PIRP pIRP);
	NTSTATUS GetNextSize(PIRP pIRP);

public:
	void LockExclusive(void)
	{
		ExAcquireResourceExclusiveLite(&m_stERES, TRUE);
	}
	
	void LockShared(void)
	{
		ExAcquireResourceSharedLite(&m_stERES, TRUE);
	}
	
	void Unlock(void)
	{
		ExReleaseResourceForThreadLite(&m_stERES, ExGetCurrentResourceThread());
	}
	
	// Virtual functions from CDevice
	virtual NTSTATUS IoControl(PIRP pIRP) 
	{
		return Standard(pIRP, IoControlCompletion);
	}

	virtual NTSTATUS Read(PIRP pIRP) 
	{
		return Standard(pIRP, ReadCompletion);
	}

	virtual NTSTATUS Write(PIRP pIRP) 
	{
		return Standard(pIRP, WriteCompletion);
	}

	virtual NTSTATUS Open(PIRP pIRP) 
	{
		return Standard(pIRP, OpenCompletion);
	}

	virtual NTSTATUS Close(PIRP pIRP) 
	{
		return Standard(pIRP, CloseCompletion);
	}

	virtual NTSTATUS Cleanup(PIRP pIRP) 
	{
		return Standard(pIRP);
	}

	virtual NTSTATUS Flush(PIRP pIRP) 
	{
		return Standard(pIRP);
	}

	virtual NTSTATUS Cancel(PIRP pIRP) 
	{
		return Standard(pIRP);
	}
};

#endif	// _DEVEXT_H_
