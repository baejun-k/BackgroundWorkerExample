#ifndef __BACKGROUND_WORKER_H__
#define __BACKGROUND_WORKER_H__

#include "IAsyncWorker.h"

#include <thread>
#include <mutex>
#include <atomic>
#include <queue>


class AutoResetEvent {
private:
	AutoResetEvent(const AutoResetEvent &);
	AutoResetEvent &operator=(const AutoResetEvent &);

	bool m_wait;
	std::mutex m_cs;
	std::condition_variable m_signal;

public:
	/// <summary>
	/// 신호가 올 때까지 기다릴 수 있게 도와 줌.
	/// </summary>
	/// <param name="initWait">생성부터 기다리게 할 지</param>
	explicit AutoResetEvent(const bool &initWait = true);

	/// <summary>
	/// Set을 해주면 다른 곳에 Wait 중인 곳을 풀어서 진행하게 함.
	/// </summary>
	void Set();

	/// <summary>
	/// 다시 Wait 상태로 초기화
	/// </summary>
	void Reset();

	/// <summary>
	/// Set 신호가 하나 올 때까지 대기함.
	/// 대기 시간을 0으로 주면 신호가 올 때까지 무한정 대기
	/// </summary>
	/// <param name="milliSec">신호가 올 때까지 대기할 시간</param>
	/// <returns>true:신호가 옴, false:신호가 아직 안 옴</returns>
	bool WaitOne(const unsigned long long &milliSec = 0);
};


class BackgroundWorker : public IAsyncWorker 
{
protected:
	const char *m_workerName;
	std::atomic<size_t> m_maxTask;
	std::atomic<size_t> m_numTask;
	std::atomic<bool> m_isBusy;
	AutoResetEvent m_inputWait;
	AutoResetEvent m_exitWait;
	std::mutex m_cs;
	std::queue<std::shared_ptr<IRunnable>> m_msgQueue;
	std::thread m_worker;

	/// <summary>
	/// 백그라운드에서 반복 실행되는 루프
	/// </summary>
	void BackgroundProc();

	bool GetFrontTask(std::shared_ptr<IRunnable> &outTask, const unsigned long long &milliSec = 10);

public:
	/// <summary>
	/// Background(비동기)로 작업 실행시켜준다.
	/// </summary>
	/// <param name="workerName">BackgroundWorker의 이름</param>
	/// <param name="queueSize">Background 작업목록의 최대 수</param>
	BackgroundWorker(const char *workerName = "", const size_t &queueSize = 100);
	~BackgroundWorker();

	/// <summary>
	/// background worker 실행.
	/// Run()하면 Abort()할 때까지 종료되지 않음.
	/// </summary>
	virtual void Run() override;
	
	/// <summary>
	/// 백그라운드에서 작업할 일을 추가
	/// </summary>
	/// <param name="runnable"></param>
	/// <returns></returns>
	virtual bool AddTask(std::shared_ptr<IRunnable> runnable) override;

	/// <summary>
	/// 현재 메시지 큐에 있는 작업을 비운다.
	/// </summary>
	virtual void ClearTask() override final;

	/// <summary>
	/// 현재 백그라운드 작업을 멈춘다.(다음 작업으로 넘어가지 않는다.)
	/// 메시지 큐는 비우지 않음.
	/// 현재 실행 중인 IRunnable가 멈추지는 않는 상태.
	/// </summary>
	virtual void Abort() override;

	/// <summary>
	/// BackgroundWorker가 Abort()된 상태(IsBusy가 false인 상태)를 기다림
	/// </summary>
	/// <param name="milliSec">종료 확인까지 대기 시간</param>
	/// <returns>true:종료됨, false:종료되지 않음</returns>
	virtual bool WaitToExit(const unsigned long long &milliSec = 0) override;

	/// <summary>
	/// BackgroundWorker가 실행 중인지
	/// </summary>
	/// <returns></returns>
	__forceinline const bool IsBusy()
	{
		return m_isBusy.load();
	}

	/// <summary>
	/// BackgroundWorker의 최대 작업목록 수
	/// </summary>
	/// <returns></returns>
	__forceinline const size_t GetQueueSize()
	{
		return m_maxTask.load();
	}

	/// <summary>
	/// 현재 작업목록에 있는 작업 수
	/// </summary>
	/// <returns></returns>
	__forceinline const size_t GetNumTask()
	{
		return m_numTask.load();
	}
};

#endif // !__BACKGROUND_WORKER_H__
