

#include "SerMon.h"
#include "DrvClass.h"

void * __cdecl operator new(size_t nSize, POOL_TYPE iType)
{
    return ExAllocatePool(iType, nSize);
}

void __cdecl operator delete(void* p)
{
    ExFreePool(p);
}

// derived class Unicode string

#define TYPE_SYSTEM_ALLOCATED	0
#define TYPE_DRIVER_ALLOCATED	1

CUString::CUString(USHORT nSize)
{
	m_status = STATUS_INSUFFICIENT_RESOURCES;
	m_bType  = TYPE_DRIVER_ALLOCATED;

	RtlInitUnicodeString(&m_String, NULL);

	m_String.MaximumLength = nSize;
	m_String.Buffer = (PWSTR)ExAllocatePool(PagedPool, nSize);
	if( m_String.Buffer )
	{
		RtlZeroMemory(m_String.Buffer, m_String.MaximumLength);
		m_status = STATUS_SUCCESS;
	}
}

CUString::CUString(PWCHAR puszString)
{
    m_status = STATUS_SUCCESS;
    m_bType  = TYPE_SYSTEM_ALLOCATED;

    RtlInitUnicodeString(&m_String, puszString);
}

CUString::~CUString()
{
	if( (m_bType==TYPE_DRIVER_ALLOCATED) && m_String.Buffer )
	{
		ExFreePool(m_String.Buffer);
	}
}

void CUString::Append(UNICODE_STRING *puszString)
{
	m_status = RtlAppendUnicodeStringToString(&m_String, puszString);
}

void CUString::CopyTo(CUString *pustrTarget)
{
	RtlCopyUnicodeString(&pustrTarget->m_String, &m_String);
}

BOOL CUString::operator==(CUString uString)
{
	return (!RtlCompareUnicodeString(&m_String, &uString.m_String, FALSE));
}
