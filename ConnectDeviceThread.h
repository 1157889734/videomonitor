#ifndef CONNECTDEVICETHREAD_H
#define CONNECTDEVICETHREAD_H

#include<QThread>
#include<GlobleDefine.h>
#include<Linux_Sdk/MMSHCNetSDK.h>

class CConnectDeviceThread : public QThread
{
    Q_OBJECT
public:
    explicit CConnectDeviceThread(QObject *parent = nullptr);
    void SetSubDevicesParamInfo();
    void InitParams( MMS::CMMSSdk* pSdk,int CarriageIndex, QMutex &m_mutex,volatile int64_t &m_HeartBeatTime){m_pSdk = pSdk;
                                                                            m_iCarriageIndex = CarriageIndex;
                                                                            m_pmutex=&m_mutex;
                                                                            m_pHeartBeatTime =&m_HeartBeatTime;}
    bool SetThreadStatus(bool bRun){bRunning =bRun;}

protected:
    virtual void run();
    int InitDevice();
    void SetSubDevicesParamInfo(int iGroupNo);
    void GetDevicesParamInfo(int iGroupNo);
    bool CloseDevice();
    bool HeartBeat();
    bool IsValidDevice();

public :

   int                                 m_iCarriageIndex;
   MMS::CMMSSdk*                       m_pSdk;
   bool                                bRunning;
   int                                 iThreadStatus;
   QMutex                              *m_pmutex;
   QMutex                              m_mutex;
   int                                 m_failCount;
   volatile int64_t                    *m_pHeartBeatTime;


signals:

public slots: 

};

#endif // CONNECTDEVICETHREAD_H
