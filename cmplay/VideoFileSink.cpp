#include "VideoFileSink.h"
#include "H264VideoRTPSource.hh"


#define   SINK_NO_EXIST			0
#define   SINK_IS_USE			1

# if 0
static char s_acSinkState[32] = {0};
static CMutexLock s_aMutexLock[32] ;

static int AllocateVideoIndex()
{
	for(int i=0;i<32;i++)
	{
		if(SINK_NO_EXIST == s_acSinkState[i])
		{
			s_acSinkState[i] = SINK_IS_USE;
			return i;
		}
	}
	return -1;
}

static int GetSinkState(int iIndex)
{
	if(iIndex<32 && iIndex>=0)
	{
		return s_acSinkState[iIndex];
	}
	return SINK_NO_EXIST;
}


static int SetSinkState(int iIndex,int iState)
{
	if(iIndex<32 && iIndex>=0)
	{
		s_acSinkState[iIndex] = iState;
		return iState;
	}
	return SINK_NO_EXIST;
}

static int LockSink(int iIndex)
{
	if(iIndex<32 && iIndex>=0)
	{
		s_aMutexLock[iIndex].Lock();
		return 0;
	}
	return -1;
}

static int UnlockSink(int iIndex)
{
	if(iIndex<32 && iIndex>=0)
	{
		s_aMutexLock[iIndex].Unlock();
		return 0;
	}
	return -1;
}

#endif

VideoFileSink* VideoFileSink::createNew(UsageEnvironment& env, MediaSubsession *pSubsession /*= NULL*/, unsigned uibufferSize /*= 800000*/)
{
	return new VideoFileSink(env, pSubsession, uibufferSize);
}

void VideoFileSink::SetRtpSetDataCBfun(PF_RTP_SET_VIDEO_DATA_CALLBACK pfRtpSetDataCB, void * pfRtpSetDataCBArg)
{
	m_CallBackFunLock.Lock();
	//LockSink(m_iIndex);
	m_pfRtpSetDataCB    = (PF_RTP_SET_VIDEO_DATA_CALLBACK)pfRtpSetDataCB;
	m_pfRtpSetDataCBArg = pfRtpSetDataCBArg;
	//m_CallBackFunLock.Unlock();
	//UnlockSink(m_iIndex);
}

void VideoFileSink::SetMediaSubSession(MediaSubsession *pSubsession)
{
	m_pSubsession = pSubsession;
}

MediaSubsession* VideoFileSink::GetMediaSubSession()
{
	return m_pSubsession;
}

VideoFileSink::VideoFileSink(UsageEnvironment& env,MediaSubsession *pSubsession ,
	unsigned uibufferSize):MediaSink(env)
{
	m_pSubsession	 = pSubsession;
	m_pfRtpSetDataCB = NULL;
	m_pfRtpSetDataCBArg = NULL;
	m_iBufferSize	 = uibufferSize;
	m_bFirstData	 = true;
	m_pszBuffer		 = new unsigned char[uibufferSize+4];
	m_pszBuffer[0]	 = 0;
	m_pszBuffer[1]   = 0;
	m_pszBuffer[2]   = 0;
	m_pszBuffer[3]   = 1;
	m_pBufferData    = new unsigned char[uibufferSize+4];
	m_PreTimets.tv_sec = 0;
	m_PreTimets.tv_usec = 0;
	m_iBufferDateLen = 0;
	//m_iIndex = AllocateVideoIndex();

}


VideoFileSink::~VideoFileSink(void)
{
	//m_CallBackFunLock.Lock();

	//LockSink(m_iIndex);
	//SetSinkState(m_iIndex,SINK_IS_USE);
	if(m_pszBuffer)
	{
		delete m_pszBuffer;
		m_pszBuffer = NULL;
	}

	if(m_pBufferData)
	{
		delete m_pBufferData;
		m_pBufferData = NULL;
	}

	//SetSinkState(m_iIndex,SINK_NO_EXIST);
	//UnlockSink(m_iIndex);

}

Boolean VideoFileSink::continuePlaying()
{
	if (fSource == NULL) return False;

	fSource->getNextFrame(m_pszBuffer+4, m_iBufferSize,
		afterGettingFrame, this,
		onSourceClosure, this);
	return True;
}

void VideoFileSink::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds)
{
	VideoFileSink* pSink = (VideoFileSink*)clientData;
	//OutputDebugStringA("afterGettingFrame 148\n");
	if(pSink)
	{
		int iIndex = pSink->m_iIndex;

		//int iRet = LockSink(iIndex);
		
		//if( 0 == iRet)
		{
			//if(SINK_IS_USE == GetSinkState(iIndex))
			{
				pSink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime);
			}		
		}
		//UnlockSink(iIndex);
	}	
	//OutputDebugStringA("afterGettingFrame 166\n");
	/*char szData[100];
	sprintf(szData,"s = %ld s,us = %ld us iDatalen = %d\r\n",presentationTime.tv_sec,presentationTime.tv_usec,frameSize);
	OutputDebugStringA(szData);*/
}

void VideoFileSink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime)
{
	if(m_bFirstData)
	{
		unsigned char const szStartCode[4] = {0x00, 0x00, 0x00, 0x01};
		if(!strcmp(m_pSubsession->mediumName(), "video") && !strcmp(m_pSubsession->codecName(), "H264"))
		{
			char const* fSPropParameterSetsStr = m_pSubsession->fmtp_spropparametersets();
			unsigned numSPropRecords;
			SPropRecord* sPropRecords = parseSPropParameterSets(fSPropParameterSetsStr, numSPropRecords);
			for (unsigned i = 0; i < numSPropRecords; ++i) 
			{
				memcpy(m_pBufferData + m_iBufferDateLen,szStartCode,4);
				memcpy(m_pBufferData + 4 + m_iBufferDateLen,sPropRecords[i].sPropBytes,sPropRecords[i].sPropLength);
				m_iBufferDateLen += 4 + sPropRecords[i].sPropLength;
			}
			delete[] sPropRecords;
		}
		else if(!strcmp(m_pSubsession->mediumName(), "video") && !strcmp(m_pSubsession->codecName(), "H265"))
		{
			char const* pszSPropParameterSetsStr[3] ;

			pszSPropParameterSetsStr[0] = m_pSubsession->fmtp_spropvps();
			pszSPropParameterSetsStr[1] = m_pSubsession->fmtp_spropsps();
			pszSPropParameterSetsStr[2] = m_pSubsession->fmtp_sproppps();
			for(int j =0;j<3 ;j++)
			{
				unsigned numSPropRecords;
				SPropRecord* sPropRecords = parseSPropParameterSets(pszSPropParameterSetsStr[j], numSPropRecords);
				for (unsigned i = 0; i < numSPropRecords; ++i) 
				{
					memcpy(m_pBufferData + m_iBufferDateLen,szStartCode,4);
					memcpy(m_pBufferData + 4 + m_iBufferDateLen,sPropRecords[i].sPropBytes,sPropRecords[i].sPropLength);
					m_iBufferDateLen += 4 + sPropRecords[i].sPropLength;
				}
				delete[] sPropRecords;
			}
		}
		m_bFirstData = FALSE;
	}
	if((m_PreTimets.tv_sec == presentationTime.tv_sec && m_PreTimets.tv_usec == presentationTime.tv_usec) || (m_PreTimets.tv_sec == 0 && m_PreTimets.tv_usec ==0 ))
	{
		memcpy(&m_pBufferData[m_iBufferDateLen],m_pszBuffer,frameSize + 4);
		m_iBufferDateLen += frameSize +4;
	}
	else
	{
		
		if(m_pfRtpSetDataCB && m_pfRtpSetDataCB)
		{
			T_FRAME_INFO tFrameInfo;
			T_VIDEO_INFO tVideoInfo;

			MediaSubsession *pSubsession =  GetMediaSubSession();

			tVideoInfo.iWidth = pSubsession->videoWidth();
			tVideoInfo.iHeight = pSubsession->videoHeight();
			tVideoInfo.pCodecName = pSubsession->codecName();

			tFrameInfo.iFrameLen = m_iBufferDateLen;
			tFrameInfo.iFrameRate = m_pSubsession->videoFPS();
			tFrameInfo.pcFrame = m_pBufferData;
			tFrameInfo.Timestamp = presentationTime;

			m_pfRtpSetDataCB(&tFrameInfo,&tVideoInfo,m_pfRtpSetDataCBArg);
		}
		
		m_iBufferDateLen = 0;
		memcpy(&m_pBufferData[m_iBufferDateLen],m_pszBuffer,frameSize + 4);
		m_iBufferDateLen += frameSize +4;
	}
	m_PreTimets = presentationTime;
	continuePlaying();
}
