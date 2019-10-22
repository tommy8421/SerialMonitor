
#include "SerMon.h"
#include "DrvClass.h"
#include "DevExt.h"

//////////////////////////////////////////////////////////////////////////
#define NAME_THIS_DEVICE		L"\\Device\\PdaAux"
#define NAME_SYMBOL_LINK		L"\\??\\PdaAux"
//////////////////////////////////////////////////////////////////////////
extern "C"
{
#ifdef ALLOC_PRAGMA
// all auxiliary routines that are called during initialization should go in here.
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, CreateDevices)
#endif
}
//////////////////////////////////////////////////////////////////////////
PDEVICE_OBJECT					g_pDeviceObj;
PDRIVER_OBJECT					g_pDriverObj;
CDBLinkedList<CAttachedDevice>	*g_pDevList;
//////////////////////////////////////////////////////////////////////////

NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pusRegistryPath)
{
	UNREFERENCED_PARAMETER(pusRegistryPath);

	g_pDeviceObj = NULL;
	g_pDriverObj = NULL;
	g_pDevList   = NULL;

	KdPrint(("DriverEntry Enter"));
	NTSTATUS	nStatus = CreateDevices(pDriverObject);
	if( NT_SUCCESS(nStatus) )
	{
		g_pDriverObj = pDriverObject;
		
		pDriverObject->MajorFunction[IRP_MJ_CREATE]  = SerMonOpen;
		pDriverObject->MajorFunction[IRP_MJ_CLOSE]   = SerMonClose;
		pDriverObject->MajorFunction[IRP_MJ_READ]    = SerMonRead;
		pDriverObject->MajorFunction[IRP_MJ_WRITE]   = SerMonWrite;
		pDriverObject->MajorFunction[IRP_MJ_CLEANUP] = SerMonCleanup;
		pDriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS]  = SerMonFlush;
		pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = SerMonIoControl;
		pDriverObject->DriverUnload = SerMonUnload;
		
		g_pDevList = new (NonPagedPool)CDBLinkedList<CAttachedDevice>;
	}
	KdPrint(("DriverEntry exit:%X", nStatus));
	
	return nStatus;
}

NTSTATUS CreateDevices(IN PDRIVER_OBJECT pDriverObject)
{
	NTSTATUS	nStatus      = STATUS_SUCCESS;
	CUString	*pusName     = NULL;   
	CUString	*pusLinkName = NULL;
	
	__try
	{
		pusName = new (PagedPool)CUString(NAME_THIS_DEVICE);
		if( !OK_ALLOCATED(pusName) )
		{
			nStatus = STATUS_NO_MEMORY;
			__leave;
		}

		nStatus = IoCreateDevice(pDriverObject,
								sizeof(CSerMonDevice *),
								&pusName->m_String,
								FILE_DEVICE_UNKNOWN,
								0,
								FALSE,
								&g_pDeviceObj);
		if( !NT_SUCCESS(nStatus) )
		{
			__leave;
		}

		g_pDeviceObj->Flags |= DO_BUFFERED_IO;
		g_pDeviceObj->DeviceExtension = new (NonPagedPool)CSerMonDevice;
		if( g_pDeviceObj->DeviceExtension==NULL )
		{
			nStatus = STATUS_NO_MEMORY;
			__leave;
		}

		pusLinkName = new (PagedPool)CUString(NAME_SYMBOL_LINK);
		if( !OK_ALLOCATED(pusLinkName) )
		{
			nStatus = STATUS_NO_MEMORY;
			__leave;
		}
		
		nStatus = IoCreateSymbolicLink (&pusLinkName->m_String, &pusName->m_String);
		if( !NT_SUCCESS(nStatus) )
		{
			__leave;
		}
	}
	__finally
	{
		if( pusName!=NULL )
		{
			delete pusName;
		}

		if( pusLinkName!=NULL )
		{
			delete pusLinkName;
		}

		if( !NT_SUCCESS(nStatus) )
		{
			if( g_pDeviceObj->DeviceExtension!=NULL )
			{
				delete g_pDeviceObj->DeviceExtension;
			}

			if( g_pDeviceObj!=NULL )
			{
				IoDeleteDevice(g_pDeviceObj);
			}
		}
	}
	
	return nStatus;
}

VOID SerMonUnload(IN PDRIVER_OBJECT pDriverObject)
{
	if( g_pDeviceObj->DeviceExtension!=NULL )
	{
		delete g_pDeviceObj->DeviceExtension;
	}
	
	if( g_pDeviceObj!=NULL )
	{
		IoDeleteDevice(g_pDeviceObj);
		g_pDeviceObj = NULL;
	}
	
	CUString	*pusLinkName = new (PagedPool)CUString(NAME_SYMBOL_LINK);
	if( OK_ALLOCATED(pusLinkName) )
	{
		IoDeleteSymbolicLink(&pusLinkName->m_String);
		delete pusLinkName;
	}
	
	if( g_pDevList!=NULL )
	{
		delete g_pDevList;    // this will kill all attached devices
		g_pDevList = NULL;
	}
}

IMPLEMENT_FUNCTION(Read)
IMPLEMENT_FUNCTION(Write)
IMPLEMENT_FUNCTION(Flush)
IMPLEMENT_FUNCTION(Cleanup)
IMPLEMENT_FUNCTION(Open)
IMPLEMENT_FUNCTION(Close)
IMPLEMENT_FUNCTION(IoControl)
