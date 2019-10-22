
#ifndef __SERMON_H_
#define __SERMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <ntddk.h>
#include <devioctl.h>

#define DECLARE_FUNCTION(func)		NTSTATUS SerMon##func(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)

#define IMPLEMENT_FUNCTION(func)	NTSTATUS SerMon##func(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp) \
												{ \
													return ((CDevice *)(pDeviceObject->DeviceExtension))->func(pIrp); \
												}


NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pusRegistryPath);
NTSTATUS CreateDevices(IN PDRIVER_OBJECT pDriverObject);
VOID SerMonUnload(IN PDRIVER_OBJECT pDriverObject);

DECLARE_FUNCTION(Open);
DECLARE_FUNCTION(Read);
DECLARE_FUNCTION(Write);
DECLARE_FUNCTION(Flush);
DECLARE_FUNCTION(Cleanup);
DECLARE_FUNCTION(Close);
DECLARE_FUNCTION(IoControl);

#ifdef __cplusplus
}
#endif

#endif	// __SERMON_H_
