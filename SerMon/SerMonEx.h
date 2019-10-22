
#ifndef _SERMON_EX_H_
#define _SERMON_EX_H_

#ifndef SERMON_DRIVER
#include <winioctl.h>    // if it's not a driver, include it manually
#endif

#define FILE_DEVICE_SERMON			0x00001001

// IOCTL_START_MONITOR initializes port monitoring, obtains a
// handle and returns it to calling application
// Input buffer contains null-terminated string naming serial
// device ("serial1","serial2"...)

// Output buffer will contain MHANDLE value that application 
// should use in all subsequent calls to this driver
#define IOCTL_SERMON_STARTMONITOR	CTL_CODE(FILE_DEVICE_SERMON, 0x0801, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Input buffer contains MHANDLE value application received
// in a call to IOCTL_START_MONITOR
#define IOCTL_SERMON_STOPMONITOR	CTL_CODE(FILE_DEVICE_SERMON, 0x0802, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Input buffer contains MHANDLE value application received
// in a call to IOCTL_START_MONITOR,
// Output buffer will contain the size of the information
#define IOCTL_SERMON_GETINFOSIZE	CTL_CODE(FILE_DEVICE_SERMON, 0x0803, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Input buffer contains MHANDLE value application received 
// in a call to IOCTL_START_MONITOR,
// Output buffer will contain the information copied
#define IOCTL_SERMON_GETINFO		CTL_CODE(FILE_DEVICE_SERMON, 0x0804, METHOD_BUFFERED, FILE_ANY_ACCESS)

// below defines is come from ntddser.h
#define IOCTL_SERIAL_SET_BAUD_RATE      CTL_CODE(FILE_DEVICE_SERIAL_PORT,  1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SERIAL_SET_QUEUE_SIZE     CTL_CODE(FILE_DEVICE_SERIAL_PORT,  2, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SERIAL_SET_LINE_CONTROL   CTL_CODE(FILE_DEVICE_SERIAL_PORT,  3, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SERIAL_SET_TIMEOUTS       CTL_CODE(FILE_DEVICE_SERIAL_PORT,  7, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SERIAL_SET_DTR            CTL_CODE(FILE_DEVICE_SERIAL_PORT,  9, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SERIAL_CLR_DTR            CTL_CODE(FILE_DEVICE_SERIAL_PORT, 10, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SERIAL_RESET_DEVICE       CTL_CODE(FILE_DEVICE_SERIAL_PORT, 11, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SERIAL_SET_RTS            CTL_CODE(FILE_DEVICE_SERIAL_PORT, 12, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SERIAL_CLR_RTS            CTL_CODE(FILE_DEVICE_SERIAL_PORT, 13, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SERIAL_PURGE              CTL_CODE(FILE_DEVICE_SERIAL_PORT, 19, METHOD_BUFFERED, FILE_ANY_ACCESS)

// for IOCTL_SERIAL_PURGE
#define SERIAL_PURGE_TXABORT			0x00000001		// Implies the current and all pending writes.
#define SERIAL_PURGE_RXABORT			0x00000002		// Implies the current and all pending reads.
#define SERIAL_PURGE_TXCLEAR			0x00000004		// Implies the transmit buffer if exists
#define SERIAL_PURGE_RXCLEAR			0x00000008		// Implies the receive buffer if exists.

#define INIT_MHANDLE(mh)		RtlZeroMemory(&mh, sizeof(mh))

typedef struct _tagMHANDLE
{
	UINT32	nData[2];
}MHANDLE, *PMHANDLE;

// Request types
enum
{
	REQ_OPEN = 1,
	REQ_CLOSE,
	REQ_READ,
	REQ_WRITE,
	REQ_SET_BAUD_RATE,
	REQ_SET_LINE_CONTROL,
	REQ_SET_QUEUE_SIZE,
	REQ_SET_TIMEOUTS,
	REQ_SERIAL_PURGE,
	REQ_SET_RTS,
	REQ_CLR_RTS,
	REQ_SET_DTR,
	REQ_CLR_DTR,
	REQ_RESET_DEVICE,
};

struct IOReq
{
	ULONG			m_nSizeRequested;
	ULONG			m_nSizeCopied;
	ULONG			m_nType;
	UCHAR			m_sPadding1[4];		// for bit 64 alignment(8 bytes)

#ifdef SERMON_DRIVER
		LIST_ENTRY	m_Entry;
		PVOID		m_pData;
	#ifndef _WIN64
		UCHAR		m_sPadding2[12];
	#endif
#else
	UCHAR			m_sPadding2[24];
#endif

#ifdef SERMON_DRIVER
	IOReq(ULONG nType, ULONG nReqSize=0, ULONG nCopySize=0, PVOID pData=NULL)
		: m_nType(nType), m_nSizeRequested(nReqSize), m_nSizeCopied(nCopySize), m_pData(NULL)
	{
		if( m_nSizeCopied )
		{
			m_pData = ExAllocatePool(NonPagedPool, m_nSizeCopied);
			if( m_pData!=NULL )
			{
				RtlCopyMemory(m_pData, pData, m_nSizeCopied);
			}
		}
	}

	~IOReq()
	{
		if( m_pData!=NULL )
		{
			ExFreePool((PVOID)m_pData);
		}
	}
#endif    // SERMON_DRIVER
};

typedef struct _tagSERIAL_BAUD_RATE
{
	ULONG	BaudRate;
}SERIAL_BAUD_RATE, *PSERIAL_BAUD_RATE;

typedef struct _tagSERIAL_LINE_CONTROL 
{
	UCHAR	StopBits;
	UCHAR	Parity;
	UCHAR	WordLength;
}SERIAL_LINE_CONTROL, *PSERIAL_LINE_CONTROL;

typedef struct _tagSERIAL_TIMEOUTS
{
	ULONG	ReadIntervalTimeout;
	ULONG	ReadTotalTimeoutMultiplier;
	ULONG	ReadTotalTimeoutConstant;
	ULONG	WriteTotalTimeoutMultiplier;
	ULONG	WriteTotalTimeoutConstant;
}SERIAL_TIMEOUTS, *PSERIAL_TIMEOUTS;

typedef struct _tagSERIAL_QUEUE_SIZE
{
	ULONG	InSize;
	ULONG	OutSize;
}SERIAL_QUEUE_SIZE, *PSERIAL_QUEUE_SIZE;

#endif    // _SERMON_EX_H_
