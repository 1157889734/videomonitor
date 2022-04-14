#include "mutex.h"
// »¥²ðËø

CMutexLock::CMutexLock()
{
    InitializeCriticalSection(&mutex);
}
CMutexLock::~CMutexLock()
{
    DeleteCriticalSection(&mutex);
}

void CMutexLock::Lock()
{
    EnterCriticalSection(&mutex);
}
void CMutexLock::Unlock()
{
    LeaveCriticalSection(&mutex);
}
