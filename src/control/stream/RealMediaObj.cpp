#include "RealMediaObj.h"
#include "FilterFactory.h"
#include "x_socket.h"
#include "x_time.h"

#define CLIENT_BUFFER_SIZE (2 * 1024 * 1024)
#define MAX_SEND_BUFF	8100

CRealMediaObj::CRealMediaObj(int nSocket, int nStreamType, J_Obj *pObj)
{
	m_nSocket = nSocket;
	m_nStreamType = nStreamType;
	m_pRingBuffer = NULL;
	m_bStart = false;
	m_pDataBuff = new char[CLIENT_BUFFER_SIZE];
	m_pConvetBuff = new char[CLIENT_BUFFER_SIZE];
	m_sendSocket.Init(nSocket);

	m_pObj = pObj;

	J_OS::LOGINFO("CRealMediaObj::CRealMediaObj created socket =  %d", m_nSocket);
}

CRealMediaObj::~CRealMediaObj()
{
	if (m_pDataBuff != NULL)
		delete m_pDataBuff;

	J_OS::LOGINFO("CRealMediaObj::~CRealMediaObj destroyed socket =  %d", m_nSocket);
}

int CRealMediaObj::Process(int nIoType)
{
	int nRet = J_OK;
	J_CommandFilter *videoCommand = dynamic_cast<J_CommandFilter *>(m_pObj);
	if (videoCommand != NULL)
	{
		if (nIoType == J_IoRead)
		{
			m_resid = videoCommand->GetResid();
			switch (videoCommand->GetCommandType())
			{
			case J_START_REAL:
				nRet = StartVideo();
				J_OS::LOGINFO("CRealMediaObj::Process StartVideo socket =  %d ret = %d", m_nSocket, nRet);
				break;
			case J_STOP_REAL:
			{
				nRet = StopVideo();
				J_OS::LOGINFO("CRealMediaObj::Process StopVideo socket =  %d ret = %d", m_nSocket, nRet);
				break;
			}
			default:
				J_OS::LOGINFO("CRealMediaObj::Process CommandType unkown type =  %d", videoCommand->GetCommandType());
				break;
			}
		}
		else if (nIoType == J_IoWrite)
		{
			if (!m_bStart)
			{
				J_OS::LOGINFO("CRealMediaObj::Process !m_bStart socket = %d", m_nSocket);
				return J_OK;
			}

			J_RequestFilter *pAccess = dynamic_cast<J_RequestFilter *>(m_pObj);
			//J_OS::LOGINFO("5555");
			memset(&m_streamHeader, 0, sizeof(m_streamHeader));
			nRet = m_pRingBuffer->PopBuffer(m_pDataBuff/* + sizeof(PackHeader)*/, m_streamHeader);
			if (nRet == J_OK && m_streamHeader.dataLen > 0)
			{
                //J_OS::LOGINFO("end %lld,%lld", m_streamHeader.timeStamp, CTime::Instance()->GetLocalTime(0));
                int nDataLen = 0;
				//m_streamHeader.timeStamp &= 0x1FFFFFF;
				pAccess->Convert(m_pDataBuff, m_streamHeader, m_pConvetBuff, nDataLen);
				if (nDataLen > 0)
				{
					int nRet = 0;
					if ((nRet = m_sendSocket.Write_n(m_pConvetBuff/* + nOffset*/, (uint32_t)nDataLen)) < 0)
					{
						J_OS::LOGERROR("CRealMediaObj::OnWrite Data error");
						return J_SOCKET_ERROR;
					}
					//J_OS::LOGINFO("%s send_len = %d", m_pConvetBuff, nRet);
				}
				else
				{
					usleep(1);
					return J_OK;
				}
			}
			else if (m_streamHeader.frameType == J_MediaBroken)
			{
			    J_OS::LOGERROR("CRealMediaObj::OnWrite Source Broken");
			    return J_SOCKET_ERROR;
			}
			else
			{
				usleep(1);
				//J_OS::LOGINFO("!m_pRingBuffer->PopBuffer socket = %d", m_nSocket);
				return J_OK;
			}
		}
	}

	return nRet;
}

int CRealMediaObj::Clearn()
{
	StopVideo();
	if (m_pObj)
	{
		CFilterFactory::Instance()->DelFilter(m_nSocket);
		m_pObj = NULL;
	}
	J_OS::LOGINFO("CRealMediaObj::OnBroken socket = %d broken", m_nSocket);

	return J_OK;
}

int CRealMediaObj::Run(bool bFlag)
{
	m_bStart = bFlag;

	return J_OK;
}

int CRealMediaObj::StartVideo()
{
	int nRet = CAdapterManager::Instance()->StartVideo(m_resid.c_str(), m_nStreamType, m_nSocket);
	if (nRet < 0)
	{
		J_OS::LOGINFO("CRealMediaObj::StartVideo StartVideo error ret = %d", nRet);
		return nRet;
	}

	nRet = CAdapterManager::Instance()->GetRingBuffer(m_resid.c_str(), m_nStreamType, m_nSocket, m_pRingBuffer);
	if (nRet < 0)
	{
		J_OS::LOGINFO("CVideoClient::StartVideo GetRingBuffer error ret = %d", nRet);
		return nRet;
	}
	//m_bStart = true;

	J_OS::LOGINFO("CRealMediaObj::StartVideo socket =  %d start", m_nSocket);

	return J_OK;
}

int CRealMediaObj::StopVideo()
{
	if (m_bStart)
	{
		m_bStart = false;
		int nRet = CAdapterManager::Instance()->StopVideo(m_resid.c_str(), m_nStreamType, m_nSocket);
		if (nRet < 0)
		{
			J_OS::LOGINFO("CRealMediaObj::StopVideo StopVideo error ret = %d", nRet);
			return nRet;
		}

		J_OS::LOGINFO("CRealMediaObj::StopVideo socket =  %d stop", m_nSocket);
	}

	return J_OK;
}

const char *CRealMediaObj::GetResid() const
{
	return m_resid.c_str();
}
