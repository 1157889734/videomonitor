#pragma once

#include <atlbase.h>

// »¥²ðËø
class CMutexLock
{
    CMutexLock(const CMutexLock &);
    CMutexLock &operator=(const CMutexLock &);
public:
	CMutexLock();
	virtual ~CMutexLock();

	void Lock();
	void Unlock();

private:
    CRITICAL_SECTION mutex;
};