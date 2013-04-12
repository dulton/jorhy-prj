#pragma once
#include "pl_player.h"
#include "BTKControl.h"
#include <string>
#include "pl_lock.h"
#include "pl_err.h"
#include "pl_factory.h"

class PL_API BTKPlayer :public PlPlayer
{
public:
	BTKPlayer(int nWorkMode, HWND hWnd);
	~BTKPlayer(void);

	static int Maker(PlPlayer *&pObj, int nWorkMode, HWND hWnd)
	{
		pObj = new BTKPlayer(nWorkMode, hWnd);
		return PL_OK;
	}

public:
	virtual BOOL Play(HWND hPlayWnd, const PL_PlayInfo &playInfo);
	virtual void Play();
	virtual void Stop();
	virtual void Pause();
	virtual void PlayOneByOne();
	virtual int GetVolume();
	virtual BOOL SetVolume(int nVolume); // nVolume:0~~100
	virtual BOOL IsPlaying();
	virtual BOOL SetSpeed(BOOL bSpeedUp);
	virtual BOOL Capture(char *path);
	virtual BOOL Record(char *path);
	virtual int GetPlayStatus();
	virtual HWND GetPlayHwnd();
	virtual BOOL VodStreamJump(const PL_PlayInfo &playInfo);
	virtual BOOL SetOSDText(int stime,char *osdText);
	virtual BOOL IsPaused();
	virtual void AspectRatio(int width=0,int height=0);
	virtual void SleepPlayer(bool bSleep);

private:
	static void BTKAPI EndCBK(void *pdata);

private:
	BTKControl	*m_player;
	std::string m_lastMrl;
	HWND m_pPlWnd;
	HWND m_hwnd;
	PlLock m_lock;
};