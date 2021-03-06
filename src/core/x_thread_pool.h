#ifndef __X_THREAD_POOL_H_
#define __X_THREAD_POOL_H_

#include "x_lock.h"
#include "j_module.h"
#include "x_singleton.h"
#include "x_thread.h"

class JO_API CThreadPool
{
public:
	CThreadPool();
	~CThreadPool();
	
public:
	///添加任务到线程池
	///@param task 将要添加的任务
	///@return 参见x_error_type.h
	int AddTask(J_Task *task);

	///创建线程池
	///@param nThreadNum 线程池中的线程数量
	///@return 参见x_error_type.h
	int Create(int nThreadNum);

	///销毁线程池
	///@return 参见x_error_type.h
	int Destroy();

private:
#ifdef WIN32
	static unsigned X_JO_API ThreadFunc(void *param)
#else
	static void *ThreadFunc(void *param)
#endif
	{
		while (true)
		{
			CThreadPool *pThis = static_cast<CThreadPool *>(param);
			pThis->OnThreadFunc();
		}
		return 0;
	}
	void OnThreadFunc();

private:
	j_queue_task m_taskQueue;
	J_OS::CTLock m_threadMutex;
	J_OS::CXCond m_threadCond;
	CJoThread m_thread;
	j_boolean_t m_bShutDown; 
};
JO_DECLARE_SINGLETON(ThreadPool)

#endif // __X_THREAD_POOL_H_
