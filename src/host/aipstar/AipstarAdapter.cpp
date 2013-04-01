#include "AipstarAdapter.h"

#include "x_adapter_manager.h"
#include "AipstarChannel.h"

CAipstarAdapter::CAipstarAdapter(j_int32_t nDevId, const j_char_t *pAddr, j_int32_t nPort, const j_char_t *pUsername, const j_char_t *pPassword)
{
	m_devHandle = NULL;
	m_status = jo_dev_broken;
	memset(m_remoteIP, 0, sizeof(m_remoteIP));
	memset(m_username, 0, sizeof(m_username));
	memset(m_password, 0, sizeof(m_password));
	m_remotePort = nPort;
	strcpy(m_remoteIP, pAddr);
	strcpy(m_username, pUsername);
	strcpy(m_password, pPassword);

	m_devHandle = TMCC_Init(TMCC_INITTYPE_CONTROL);
	//assert(m_devHandle != NULL);
	Login();
	TMCC_RegisterConnectCallBack(m_devHandle, OnConnectCallBack, this);
	TMCC_SetAutoReConnect(m_devHandle, true);

	J_OS::LOGINFO("CAipstarAdapter::CAipstarAdapter(ip = %s, port = %d)", pAddr, nPort);
}

CAipstarAdapter::~CAipstarAdapter()
{
	if (Logout() == J_OK)
		m_status = jo_dev_broken;

	if (m_devHandle != NULL)
		TMCC_Done(m_devHandle);

	J_OS::LOGINFO("CAipstarAdapter::~CAipstarAdapter()");
}

J_DevStatus CAipstarAdapter::GetStatus() const
{
	return m_status;
}
j_result_t CAipstarAdapter::Broken()
{
	return J_OK;
}

j_result_t CAipstarAdapter::MakeChannel(const j_char_t *pResid, j_void_t *&pObj, j_void_t *pOwner, j_int32_t nChannel, j_int32_t nStream, j_int32_t nMode)
{
	CAipstarChannel *pChannel = new CAipstarChannel(pResid, pOwner, nChannel, nStream, nMode);
	if (NULL == pChannel)
		return J_MEMORY_ERROR;

	pObj = pChannel;

	return J_OK;
}

j_result_t CAipstarAdapter::Relogin()
{
    Logout();
    return Login();
}

j_result_t CAipstarAdapter::Login()
{
	tmConnectInfo_t conInfo = {0};
	conInfo.dwSize = sizeof(tmConnectInfo_t);
	strcpy(conInfo.pIp, m_remoteIP);
	conInfo.iPort = m_remotePort;
	strcpy(conInfo.szUser, m_username);
	strcpy(conInfo.szPass, m_password);

	int nRet = TMCC_Connect(m_devHandle, &conInfo, true);
	//assert(nRet == TMCC_ERR_SUCCESS);
	if (nRet == TMCC_ERR_SUCCESS)
    {
        m_status = jo_dev_ready;
    }
    else
    {
		sleep(1);
		Login();
        J_OS::LOGINFO("CAipstarAdapter::Login() Login faild");
    }

	return J_OK;
}

j_result_t CAipstarAdapter::Logout()
{
	int nRet = TMCC_DisConnect(m_devHandle);
	if (nRet == TMCC_ERR_SUCCESS)
        m_status = jo_dev_broken;

	return J_OK;
}

j_void_t CAipstarAdapter::OnConnect(HANDLE hHandle, BOOL bConnect)
{
	printf("connect = %d\n", bConnect);
}
