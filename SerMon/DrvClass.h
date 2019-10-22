

#ifndef __DRVCLASS_H_
#define __DRVCLASS_H_

#define BOOL	int
#define FALSE	0

#define OK_ALLOCATED(obj)	((obj!=(void *)0) && NT_SUCCESS((obj)->m_status))

void * __cdecl operator new(size_t nSize, POOL_TYPE iType);
void __cdecl operator delete(void* p);

#pragma warning(disable : 4996)
class CUString 
{
public:
	CUString(USHORT nSize);
	CUString(PWCHAR puszString);
	~CUString();

private:
	UCHAR			m_bType;
	
public:
	UNICODE_STRING	m_String;
	NTSTATUS		m_status;
	
public:
	void Append(UNICODE_STRING *puszString);
	void CUString::CopyTo(CUString *pstrString);
	BOOL operator==(CUString cuArg);
	
public:
	int inline GetLength()
	{
		return m_String.Length;
	}
	
	PWCHAR inline GetString()
	{
		return m_String.Buffer;
	}
	
	void inline SetLength(USHORT nLength)
	{
		m_String.Length = nLength;
	}
};

template <class T>
class CDBLinkedList
{
protected:
	LIST_ENTRY		m_Head;
	KSPIN_LOCK		m_Splock;
	
public:
	CDBLinkedList()
	{
		InitializeListHead(&m_Head);
		KeInitializeSpinLock(&m_Splock);
	}
    
	BOOL IsEmpty(void)
	{
		return IsListEmpty(&m_Head);
	}
	
	~CDBLinkedList()
	{    // if list is still not empty, free all items
		T	*p;

		while( p = (T *)ExInterlockedRemoveHeadList(&m_Head, &m_Splock) )
		{
			delete CONTAINING_RECORD(p, T, m_Entry);
		}
	}
	
	void New(T *p)
	{
		ExInterlockedInsertTailList(&m_Head, &(p->m_Entry), &m_Splock);
	}
	
	void InsertHead(T *p)
	{
		ExInterlockedInsertHeadList(&m_Head, &(p->m_Entry), &m_Splock);
	}
	
	T *RemoveHead(void)
	{
		T	*p = (T *)ExInterlockedRemoveHeadList(&m_Head, &m_Splock);

		if( p )
		{
			p = CONTAINING_RECORD(p, T, m_Entry);
		}

		return p;
	}
	
	void Remove(T *p)
	{
		RemoveEntryList(&(p->m_Entry));
	}
};

#endif	// __DRVCLASS_H_
