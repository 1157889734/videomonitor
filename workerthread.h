#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QThread>
#include <QMutex>

#ifndef nullptr
#define nullptr 0
#endif

class MutexLocker
{
public:
    MutexLocker(QMutex *pMutex, bool block = true);
    ~MutexLocker();
    inline void lock()
    {
        if(m_pQMutex)
            m_pQMutex->lock();
    }
    inline void unlock()
    {
        if(m_pQMutex)
            m_pQMutex->unlock();
    }
private:
    QMutex          *m_pQMutex;
};

typedef unsigned int(*ThreadFunc)(void *param);
class WorkerThread : public QThread
{
    Q_OBJECT
public:
    WorkerThread(QObject *parent = 0);
    void setThreadFunc(ThreadFunc pfn,void *param);
protected:
    virtual void run();
private:
    ThreadFunc   m_pfn;
    void        *m_param;
};

#endif // WORKERTHREAD_H
