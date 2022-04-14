#include "workerthread.h"
//
MutexLocker::MutexLocker(QMutex *pMutex, bool block/* = true */)
{
    m_pQMutex = pMutex;
    if(block)
    {
        lock();
    }
}

MutexLocker::~MutexLocker()
{
    unlock();
}

// WorkerThread
WorkerThread::WorkerThread(QObject *parent) : QThread(parent)
{
    m_pfn = nullptr;
    m_param = nullptr;
}
void WorkerThread::run()
{
    if(m_pfn != nullptr)
        m_pfn(m_param);
}
void WorkerThread::setThreadFunc(ThreadFunc pfn,void *param)
{
    m_pfn = pfn;
    m_param = param;
}
