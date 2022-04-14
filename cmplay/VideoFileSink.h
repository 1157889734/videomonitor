#pragma once
#include "mediasink.hh"
#include "MediaSession.hh"
#include "mutex.h"
#include "CommonDefine.h"


class VideoFileSink :public MediaSink
{
public:
	static VideoFileSink* createNew(UsageEnvironment& env, MediaSubsession *pSubsession = NULL,
		unsigned uibufferSize = 800000);
	void   SetRtpSetDataCBfun(PF_RTP_SET_VIDEO_DATA_CALLBACK pfRtpSetDataCB, void *pfRtpSetDataCBArg);
	void   SetMediaSubSession(MediaSubsession *pSubsession);
	MediaSubsession*   GetMediaSubSession();

	VideoFileSink(UsageEnvironment& env,MediaSubsession *pSubsession ,
		unsigned uibufferSize = 800000);
	~VideoFileSink(void);

protected:
	virtual Boolean continuePlaying();
	void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime);
	static void afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds);

	MediaSubsession			   *m_pSubsession;
	PF_RTP_SET_VIDEO_DATA_CALLBACK	m_pfRtpSetDataCB;
	void					   *m_pfRtpSetDataCBArg;
	unsigned char			   *m_pszBuffer;
	unsigned					m_iBufferSize;
	bool						m_bFirstData;
	CMutexLock					m_CallBackFunLock;
	unsigned char  			   *m_pBufferData ;
	timeval						m_PreTimets;
	int							m_iBufferDateLen;
	int							m_iIndex;
};

