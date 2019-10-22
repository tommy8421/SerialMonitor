
#include "SerMon.h"
#include "DrvClass.h"
#include "DevExt.h"

//////////////////////////////////////////////////////////////////////////
#define _IoControlCode(pIOStack)			pIOStack->Parameters.DeviceIoControl.IoControlCode
#define _InputBufferLength(pIOStack)		pIOStack->Parameters.DeviceIoControl.InputBufferLength
#define _OutputBufferLength(pIOStack)		pIOStack->Parameters.DeviceIoControl.OutputBufferLength
//////////////////////////////////////////////////////////////////////////

CSerMonDevice::CSerMonDevice()
{
}

CSerMonDevice::~CSerMonDevice()
{
}

NTSTATUS CSerMonDevice::Attach(PUNICODE_STRING puszDevName, DWORD32 dwDesiredAccess, PDEVICE_OBJECT *pRetDevObj)
{
	NTSTATUS         nStatus      = STATUS_SUCCESS;
	PDEVICE_OBJECT   pNewDevObj   = NULL;
	CAttachedDevice *pAttachedDev = NULL;
	BOOLEAN          fNeedUnLock  = FALSE;
	PDEVICE_OBJECT   pDevObj      = NULL;
	PFILE_OBJECT     pFileObj     = NULL;
	
	__try
	{
		KdPrint(("Attach to serial :%ws", puszDevName->Buffer));
		nStatus = IoGetDeviceObjectPointer(puszDevName, dwDesiredAccess, &pFileObj, &pDevObj);
		if( !NT_SUCCESS(nStatus) )
		{
			__leave;
		}
		
		// Create a new device object.
		nStatus = IoCreateDevice(g_pDriverObj,
								sizeof(CAttachedDevice *),
								NULL,					// unnamed object
								pDevObj->DeviceType,
								pDevObj->Characteristics,
								FALSE,					// Not exclusive.
								&pNewDevObj);
		if( !NT_SUCCESS(nStatus) )
		{
			__leave;
		}
		
		pNewDevObj->Flags &= ~DO_DEVICE_INITIALIZING;
		
		// Initialize the extension for the device object.
		pNewDevObj->DeviceExtension = pAttachedDev = new (NonPagedPool)CAttachedDevice;
		if( pAttachedDev==NULL )
		{
			nStatus = STATUS_NO_MEMORY;
			__leave;
		}

		KdPrint(("Allocated CAttachedDevice pointer:%p", pAttachedDev));
		pAttachedDev->m_pOrgDevice  = pDevObj;
		pAttachedDev->m_pThisDevice = pNewDevObj;
		
		pAttachedDev->LockExclusive();
		fNeedUnLock = TRUE;
		// attach to the target FSD.
		nStatus = IoAttachDeviceByPointer(pNewDevObj, pDevObj);
		
		ASSERT(NT_SUCCESS(nStatus));
		
		pNewDevObj->Flags |= DO_BUFFERED_IO;
	}
	__finally
	{
		if( fNeedUnLock )
		{
			pAttachedDev->Unlock();
		}
		
		if( NT_SUCCESS(nStatus) )
		{
			*pRetDevObj = pNewDevObj;
		}
		else
		{
			if( pAttachedDev!=NULL )
			{
				delete pAttachedDev;
			}

			if( pNewDevObj!=NULL )
			{
				IoDeleteDevice(pNewDevObj);
			}
		}
		
		if( pFileObj!=NULL )
		{
			ObDereferenceObject(pFileObj);
		}
	}
	
	return nStatus;
}

BOOL CSerMonDevice::ConnectToSerialDevice(LPCTSTR lpszBufferName, PMHANDLE pMHandle)
{
	PDEVICE_OBJECT	pDevObj;
	CUString		strName((PWCHAR)lpszBufferName);

	NTSTATUS	nStatus = Attach(&strName.m_String, FILE_ALL_ACCESS, &pDevObj);
	KdPrint(("Attach to serial driver return:%d\n", nStatus));
    if( NT_SUCCESS(nStatus) )
	{
#ifdef _WIN64
		RtlCopyMemory(pMHandle, &pDevObj->DeviceExtension, sizeof(MHANDLE));
#else
		pMHandle->nData[0] = (UINT32)pDevObj->DeviceExtension;
		pMHandle->nData[1] = 0;
#endif
        return TRUE;
	}

	return FALSE;
}

CAttachedDevice *CSerMonDevice::GetAttachedDevice(PVOID pIrpBuffer)
{
#ifdef _WIN64
	CAttachedDevice	*pAttachedDev;

	RtlCopyMemory(&pAttachedDev, pIrpBuffer, sizeof(MHANDLE));
	KdPrint(("GetAttachedDevice pointer:%p\n", pAttachedDev));

	return pAttachedDev;
#else
	MHANDLE			hSerMon;

	RtlCopyMemory(&hSerMon, pIrpBuffer, sizeof(MHANDLE));
	KdPrint(("GetAttachedDevice pointer:%p\n", hSerMon.nData[0]));

	return (CAttachedDevice *)hSerMon.nData[0];
#endif
}

BOOL CSerMonDevice::IsValidDevice( CAttachedDevice *pDevice )
{
	if( pDevice==NULL )
	{
		return FALSE;
	}

	return pDevice->CheckValid();
}

// 处理应用的io指令
NTSTATUS CSerMonDevice::IoControl(PIRP pIRP)
{
	PIO_STACK_LOCATION	pCurIOStack;
	CAttachedDevice		*pAttachedDev;
	MHANDLE				hSerMon;
	BOOL				fSuccess;

	pIRP->IoStatus.Information = 0;		// initialize to no information return

	pCurIOStack = IoGetCurrentIrpStackLocation(pIRP);
	switch( _IoControlCode(pCurIOStack) )
	{
	case  IOCTL_SERMON_STARTMONITOR:
		KdPrint(("IoControl IOCTL_SERMON_STARTMONITOR:%ws\n", pIRP->AssociatedIrp.SystemBuffer));
		fSuccess = ConnectToSerialDevice((LPCTSTR)pIRP->AssociatedIrp.SystemBuffer, &hSerMon);
		if( fSuccess )
		{
			RtlCopyMemory(pIRP->AssociatedIrp.SystemBuffer, &hSerMon, sizeof(MHANDLE));
			pIRP->IoStatus.Information = sizeof(MHANDLE);
			pIRP->IoStatus.Status      = STATUS_SUCCESS;
		}
		else
		{
			pIRP->IoStatus.Status      = STATUS_INVALID_PARAMETER;
		}
		break;

	case IOCTL_SERMON_STOPMONITOR:
		KdPrint(("IoControl IOCTL_SERMON_STOPMONITOR\n"));
		if( _InputBufferLength(pCurIOStack)==sizeof(MHANDLE) )
		{
			pAttachedDev = GetAttachedDevice(pIRP->AssociatedIrp.SystemBuffer);
			if( IsValidDevice(pAttachedDev) )
			{
				pIRP->IoStatus.Status = STATUS_SUCCESS;
				delete pAttachedDev;
			}
			else
			{
				pIRP->IoStatus.Status = STATUS_INVALID_PARAMETER;
			}
		}
		else
		{
			pIRP->IoStatus.Status = STATUS_INVALID_HANDLE;
		}
		break;

	case IOCTL_SERMON_GETINFOSIZE:
		KdPrint(("IoControl IOCTL_SERMON_GETINFOSIZE\n"));
		if( _InputBufferLength(pCurIOStack)==sizeof(MHANDLE) )
		{
			pAttachedDev = GetAttachedDevice(pIRP->AssociatedIrp.SystemBuffer);
			if( IsValidDevice(pAttachedDev)                     &&
				_OutputBufferLength(pCurIOStack)==sizeof(ULONG) )
			{
				return pAttachedDev->GetNextSize(pIRP);
			}
			else
			{
				pIRP->IoStatus.Status = STATUS_INVALID_PARAMETER;
			}
		}
		else
		{
			pIRP->IoStatus.Status = STATUS_INVALID_HANDLE;
		}
		break;

	case IOCTL_SERMON_GETINFO:
		KdPrint(("IoControl IOCTL_SERMON_GETINFO\n"));
		if( _InputBufferLength(pCurIOStack)==sizeof(MHANDLE) )
		{
			pAttachedDev = GetAttachedDevice(pIRP->AssociatedIrp.SystemBuffer);
			if( IsValidDevice(pAttachedDev) )
			{
				return pAttachedDev->GetNext(pIRP);
			}
			else
			{
				pIRP->IoStatus.Status = STATUS_INVALID_PARAMETER;
			}
		}
		else
		{
			pIRP->IoStatus.Status = STATUS_INVALID_HANDLE;
		}
		break;

	default:
		KdPrint(("Unsupported IoControl for CSerMonDevice:%08X\n", _IoControlCode(pCurIOStack)));
		pIRP->IoStatus.Status = STATUS_IO_DEVICE_ERROR;
		break;
	}
	KdPrint(("IoControl return:%X\n", pIRP->IoStatus.Status));

	IoCompleteRequest(pIRP, IO_NO_INCREMENT);
	return pIRP->IoStatus.Status;
}

NTSTATUS DefaultCompletion(IN PDEVICE_OBJECT pDevObj, IN PIRP pIRP, IN PVOID pContext)
{
	if( pIRP->PendingReturned )
	{
		IoMarkIrpPending(pIRP);
	}

	// Nothing to do, simply return success
	return STATUS_SUCCESS;
}

NTSTATUS ReadCompletion(IN PDEVICE_OBJECT pDevObj, IN PIRP pIRP, IN PVOID pContext)
{
	if( pIRP->PendingReturned )
	{
		IoMarkIrpPending(pIRP);
	}

	if( pIRP->IoStatus.Information!=0 )
	{
		// There are bytes read, construct IOReq item and append it to the list
		PIO_STACK_LOCATION	pCurIOStack = IoGetCurrentIrpStackLocation(pIRP);
		IOReq	*pIOReq = new (NonPagedPool)IOReq(REQ_READ,
												pCurIOStack->Parameters.Read.Length,
												(ULONG)pIRP->IoStatus.Information,
												pIRP->AssociatedIrp.SystemBuffer);
		((CAttachedDevice *)pContext)->New(pIOReq);
	}

	return STATUS_SUCCESS;
}

NTSTATUS WriteCompletion(IN PDEVICE_OBJECT pDevObj, IN PIRP pIRP, IN PVOID pContext)
{
	if( pIRP->PendingReturned )
	{
		IoMarkIrpPending(pIRP);
	}

	if( pIRP->IoStatus.Status==STATUS_SUCCESS &&
		pIRP->IoStatus.Information!=0         )
	{
		PIO_STACK_LOCATION	pCurIOStack = IoGetCurrentIrpStackLocation(pIRP);
		IOReq	*pIOReq = new (NonPagedPool)IOReq(REQ_WRITE,
												pCurIOStack->Parameters.Read.Length,
												(ULONG)pIRP->IoStatus.Information,
												pIRP->AssociatedIrp.SystemBuffer);
		((CAttachedDevice *)pContext)->New(pIOReq);
	}

	return STATUS_SUCCESS;
}

NTSTATUS OpenCompletion(IN PDEVICE_OBJECT pDevObj, IN PIRP pIRP, IN PVOID pContext)
{
	if( pIRP->PendingReturned )
	{
		IoMarkIrpPending(pIRP);
	}

	if( pIRP->IoStatus.Status==STATUS_SUCCESS )
	{
		CAttachedDevice	*pAttachedDev = (CAttachedDevice *)pContext;
		// Increase usage count
		if( InterlockedIncrement(&pAttachedDev->m_nNum) )
		{
			KeResetEvent(&pAttachedDev->m_kEvent);
		}

		pAttachedDev->New(new (NonPagedPool)IOReq(REQ_OPEN));
	}

	return STATUS_SUCCESS;
}

NTSTATUS CloseCompletion(IN PDEVICE_OBJECT pDevObj, IN PIRP pIRP, IN PVOID pContext)
{
	if( pIRP->PendingReturned )
	{
		IoMarkIrpPending(pIRP);
	}

	if( pIRP->IoStatus.Status==STATUS_SUCCESS )
	{
		CAttachedDevice	*pAttachedDev = (CAttachedDevice *)pContext;
		if( !pAttachedDev->m_bFirstTime )
		{
			// Decrease usage count and signal the m_kEvent if it falls to zero
			if( !InterlockedDecrement(&pAttachedDev->m_nNum) )
			{
				KeSetEvent(&pAttachedDev->m_kEvent, 0, FALSE);
			}

			pAttachedDev->New(new (NonPagedPool)IOReq(REQ_CLOSE));
		}
		else
		{
			pAttachedDev->m_bFirstTime = FALSE;
		}
	}

	return STATUS_SUCCESS;
}

// 处理IO指令
NTSTATUS IoControlCompletion(IN PDEVICE_OBJECT pDevObj, IN PIRP pIRP, IN PVOID pContext)
{
	static struct
	{
		ULONG	ulIoCtlCode;
		ULONG	ulIoReqType;
	}
	arIOReqList[] =
	{
		{IOCTL_SERIAL_SET_RTS,		REQ_SET_RTS},
		{IOCTL_SERIAL_CLR_RTS,		REQ_CLR_RTS},
		{IOCTL_SERIAL_SET_DTR,		REQ_SET_DTR},
		{IOCTL_SERIAL_CLR_DTR,		REQ_CLR_DTR},
		{IOCTL_SERIAL_RESET_DEVICE,	REQ_RESET_DEVICE},
	};
	int	nCnt;

	if( pIRP->PendingReturned )
	{
		IoMarkIrpPending(pIRP);
	}

	if( pIRP->IoStatus.Status==STATUS_SUCCESS )
	{
		IOReq				*pIOReq     = NULL;
		PIO_STACK_LOCATION	pCurIOStack = IoGetCurrentIrpStackLocation(pIRP);

		switch( _IoControlCode(pCurIOStack) )
		{
		case IOCTL_SERIAL_SET_BAUD_RATE:
			KdPrint(("Call IoControlCompletion: IOCTL_SERIAL_SET_BAUD_RATE,SystemBuffer=%p\n", pIRP->AssociatedIrp.SystemBuffer));
			pIOReq = new (NonPagedPool)IOReq(REQ_SET_BAUD_RATE,
										sizeof(SERIAL_BAUD_RATE),
										sizeof(SERIAL_BAUD_RATE),
										pIRP->AssociatedIrp.SystemBuffer);
			break;

		case IOCTL_SERIAL_SET_LINE_CONTROL:
			KdPrint(("Call IoControlCompletion: IOCTL_SERIAL_SET_LINE_CONTROL,SystemBuffer=%p\n", pIRP->AssociatedIrp.SystemBuffer));
			pIOReq = new (NonPagedPool)IOReq(REQ_SET_LINE_CONTROL,
										sizeof(SERIAL_LINE_CONTROL),
										sizeof(SERIAL_LINE_CONTROL),
										pIRP->AssociatedIrp.SystemBuffer);
			break;

		case IOCTL_SERIAL_SET_QUEUE_SIZE:
			KdPrint(("Call IoControlCompletion: IOCTL_SERIAL_SET_QUEUE_SIZE,SystemBuffer=%p\n", pIRP->AssociatedIrp.SystemBuffer));
			pIOReq = new (NonPagedPool)IOReq(REQ_SET_QUEUE_SIZE,
										sizeof(SERIAL_QUEUE_SIZE),
										sizeof(SERIAL_QUEUE_SIZE),
										pIRP->AssociatedIrp.SystemBuffer);
			break;

		case IOCTL_SERIAL_SET_TIMEOUTS:
			KdPrint(("Call IoControlCompletion: IOCTL_SERIAL_SET_TIMEOUTS,SystemBuffer=%p\n", pIRP->AssociatedIrp.SystemBuffer));
			pIOReq = new (NonPagedPool)IOReq(REQ_SET_TIMEOUTS,
										sizeof(SERIAL_TIMEOUTS),
										sizeof(SERIAL_TIMEOUTS),
										pIRP->AssociatedIrp.SystemBuffer);
			break;

		case IOCTL_SERIAL_PURGE:
			KdPrint(("Call IoControlCompletion: IOCTL_SERIAL_PURGE,SystemBuffer=%p\n", pIRP->AssociatedIrp.SystemBuffer));
			pIOReq = new (NonPagedPool)IOReq(REQ_SERIAL_PURGE,
										sizeof(ULONG),
										sizeof(ULONG),
										pIRP->AssociatedIrp.SystemBuffer);
			break;

		case IOCTL_SERIAL_SET_RTS:
		case IOCTL_SERIAL_CLR_RTS:
		case IOCTL_SERIAL_SET_DTR:
		case IOCTL_SERIAL_CLR_DTR:
		case IOCTL_SERIAL_RESET_DEVICE:
			KdPrint(("Call IoControlCompletion code:%08x,SystemBuffer=%p\n", _IoControlCode(pCurIOStack), pIRP->AssociatedIrp.SystemBuffer));
			for(nCnt=0; nCnt<sizeof(arIOReqList)/sizeof(arIOReqList[0]); nCnt++)
			{
				if( arIOReqList[nCnt].ulIoCtlCode==_IoControlCode(pCurIOStack) )
				{
					pIOReq = new (NonPagedPool)IOReq(arIOReqList[nCnt].ulIoReqType);
					break;
				}
			}
			break;
			
		default:
			KdPrint(("Unsupported IoControlcode=%08X, skipping....\n", _IoControlCode(pCurIOStack)));
			break;
		}

		((CAttachedDevice *)pContext)->New(pIOReq);
	}

	return STATUS_SUCCESS;
}

//////////////////////////////
// CAttachedDevice
CAttachedDevice::CAttachedDevice()
{
	m_Signature[0] = L'P';
	m_Signature[1] = L'D';
	m_Signature[2] = L'A';
	
	m_nNum       = 0;
	m_bFirstTime = TRUE;
	
	ExInitializeResourceLite(&m_stERES);
	KeInitializeEvent(&m_kEvent, NotificationEvent, TRUE);
	
	g_pDevList->New(this);
}

CAttachedDevice::~CAttachedDevice()
{
	// The target device MUST be closed so much times it was opened
	// so we don't return until this is met
	// 		KeWaitForSingleObject(&m_kEvent, Executive, KernelMode, FALSE, NULL);
	IoDetachDevice(m_pOrgDevice);
	IoDeleteDevice(m_pThisDevice);
	ExDeleteResourceLite(&m_stERES);
	
	while( 1 )
	{
		PExIRP pExIRP = m_listPending.RemoveHead();
		if( pExIRP==NULL )
		{
			break;
		}
		
		pExIRP->m_pIRP->IoStatus.Information = 0;
		pExIRP->m_pIRP->IoStatus.Status      = STATUS_CANCELLED;
		
		IoCompleteRequest(pExIRP->m_pIRP, IO_NO_INCREMENT);
		delete pExIRP;
	}
	
	g_pDevList->Remove(this);
}

// This function forwards the request
NTSTATUS CAttachedDevice::Standard(PIRP pIRP, PIO_COMPLETION_ROUTINE pfRoutine)
{
	PIO_STACK_LOCATION	pCurIOStack, pNextIOStack;

	pCurIOStack   = IoGetCurrentIrpStackLocation(pIRP);
	pNextIOStack  = IoGetNextIrpStackLocation(pIRP);	// lower level driver's IO stack
	RtlCopyMemory(pNextIOStack, pCurIOStack, sizeof(IO_STACK_LOCATION));

	IoSetCompletionRoutine(pIRP,
							pfRoutine!=NULL ? pfRoutine : DefaultCompletion,
							this,
							TRUE,
							TRUE,
							TRUE);

	KdPrint(("Call IoCallDriver, Systembuffer=%p", pIRP->AssociatedIrp.SystemBuffer));
	return IoCallDriver(m_pOrgDevice, pIRP);
}

BOOL CAttachedDevice::CheckValid()
{
	return ( MmIsAddressValid(this) &&
			 m_Signature[0]==L'P'   &&
			 m_Signature[1]==L'D'   &&
			 m_Signature[2]==L'A' );
}

void CAttachedDevice::New(IOReq *pIOReq)
{
	KdPrint(("Call CAttachedDevice::New, in pIOReq=%p\n", pIOReq));
	if( pIOReq==NULL )
	{
		return;
	}

	LockExclusive();
	m_listIoreq.New(pIOReq);

	// check for m_listPending requests
	PExIRP	pExIRP = m_listPending.RemoveHead();
	KdPrint(("Try to get m_listPending, pExIRP=%p\n", pExIRP));

	// If we have a m_listPending request, waiting for something to receive, so simply handle it
	if( pExIRP!=NULL )
	{
		IRP		*pIRP = pExIRP->m_pIRP;

		pIOReq = m_listIoreq.RemoveHead();
		Unlock();    // we don't need to lock anymore

		// process the request
		PIO_STACK_LOCATION	pCurIOStack = IoGetCurrentIrpStackLocation(pIRP);
		switch( _IoControlCode(pCurIOStack) )
		{
		case IOCTL_SERMON_GETINFOSIZE:
			LockExclusive();
			ProcessSize(pIRP, pIOReq);
			Unlock();
			break;

		case IOCTL_SERMON_GETINFO:
			ProcessNext(pIRP, pIOReq);
			break;

		default:
			KdPrint(("CAttachedDevice::New, can NEVER be here: _IoControlCode=%08X\n", _IoControlCode(pCurIOStack)));
			break;
		}

		delete pExIRP;
	}
	else
	{
		Unlock();
	}
}

NTSTATUS CAttachedDevice::ProcessSize(PIRP pIRP, IOReq *pIOReq)
{
	KdPrint(("Call ProcessSize,SystemBuffer=%p\n", pIRP->AssociatedIrp.SystemBuffer));

	pIRP->IoStatus.Information = sizeof(ULONG);
	pIRP->IoStatus.Status      = STATUS_SUCCESS;

	*((ULONG *)pIRP->AssociatedIrp.SystemBuffer) = sizeof(IOReq) + pIOReq->m_nSizeCopied;
	m_listIoreq.InsertHead(pIOReq);

	IoCompleteRequest(pIRP, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS CAttachedDevice::ProcessNext(PIRP pIRP, IOReq *pIOReq)
{
	NTSTATUS			nStatus;
	PIO_STACK_LOCATION	pCurIOStack;

	KdPrint(("call CAttachedDevice::ProcessNext, SystemBuffer=%p\n", pIRP->AssociatedIrp.SystemBuffer));

	pCurIOStack = IoGetCurrentIrpStackLocation(pIRP);
	if( _OutputBufferLength(pCurIOStack)<sizeof(IOReq)+pIOReq->m_nSizeCopied )
	{
		nStatus = STATUS_BUFFER_TOO_SMALL;
	}
	else
	{
		nStatus = STATUS_SUCCESS;
		pIRP->IoStatus.Information = sizeof(IOReq) + pIOReq->m_nSizeCopied;

		RtlCopyMemory(pIRP->AssociatedIrp.SystemBuffer, pIOReq, sizeof(IOReq));
		if( pIOReq->m_pData )
		{
			RtlCopyMemory((PCHAR)pIRP->AssociatedIrp.SystemBuffer+sizeof(IOReq), pIOReq->m_pData, pIOReq->m_nSizeCopied);
		}
	}

	KdPrint(("ProcessNext return:%X, release pIOReq=%p\n", nStatus, pIOReq));

	delete pIOReq;
	pIRP->IoStatus.Status = nStatus;
	IoCompleteRequest(pIRP, IO_NO_INCREMENT);

	return nStatus;
}

NTSTATUS CAttachedDevice::GetNextSize(PIRP pIRP)
{
	LockExclusive();
	if( !m_listIoreq.IsEmpty() )
	{
		IOReq		*pIOReq = m_listIoreq.RemoveHead();
		NTSTATUS	nStatus = ProcessSize(pIRP, pIOReq);

		Unlock();

		return nStatus;
	}
	else
	{
		ProcessEmpty(pIRP);
		Unlock();

		return STATUS_PENDING;
	}
}

NTSTATUS CAttachedDevice::GetNext(PIRP pIRP)
{
	LockExclusive();
	if( !m_listIoreq.IsEmpty() )
	{
		IOReq	*pIOReq = m_listIoreq.RemoveHead();
		Unlock();
		return ProcessNext(pIRP, pIOReq);
	}
	else
	{
		ProcessEmpty(pIRP);
		Unlock();

		return STATUS_PENDING;
	}
}

void CAttachedDevice::ProcessEmpty(PIRP pIRP)
{
	PExIRP	pExIRP = new (NonPagedPool)ExIRP;

	pExIRP->m_pIRP = pIRP;
	m_listPending.New(pExIRP);

	pIRP->IoStatus.Status = STATUS_PENDING;
	IoMarkIrpPending(pIRP);
}
