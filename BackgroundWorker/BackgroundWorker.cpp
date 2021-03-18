#include "BackgroundWorker.h"
#include "DebugUtils.h"



void BackgroundWorker::BackgroundProc()
{
	DebugOut("BackgroundWorker[%s] started\n", m_workerName);
	// Run() 중이라면 계속 실행
	while ( IsBusy() )
	{
		std::shared_ptr<IRunnable> pRunnable = nullptr;
		while ( GetFrontTask(pRunnable) )
		{
			DebugOut("BackgroundWorker[%s] Recieve Task(%lu)\n",
							 m_workerName, (size_t)pRunnable.get());
			pRunnable->Run();
		}
		m_inputWait.Reset();

		// // message queue에 작업이 추가된 적이 없다면 들어 올 때까지 대기
		// // 중간에 종료될 수도 있기 때문에 10 milli seconds마다 상태 확인을 위해 갱신
		// if ( m_inputWait.WaitOne(10) )
		// {
		// 	// message queue에 작업이 들어온 후 Run() 상태면 queue가 빌 때까지 반복.
		// 	do
		// 	{
		// 		// 실행 할 작업을 참조할 포인터
		// 		std::shared_ptr<IRunnable> pRunnable = nullptr;
		// 
		// 		{
		// 			// queue에서 작업을 뺄 때 크리티컬세션을 건다
		// 			std::lock_guard<std::mutex> _(m_cs);
		// 			// 작업이 없으면 현재 루프 종료
		// 			if ( !IsBusy() || m_msgQueue.empty() )
		// 			{
		// 				break;
		// 			}
		// 			// queue에 처음 들어온 작업을 꺼낸다.
		// 			pRunnable = m_msgQueue.front();
		// 			m_msgQueue.pop();
		// 			// 현재 작업 수 카운트 -1
		// 			m_numTask.fetch_sub(1);
		// 		}
		// 
		// 		// 꺼낸 작업이 있으면 작업 실행.
		// 		if ( nullptr != pRunnable )
		// 		{
		// 			DebugOut("BackgroundWorker[%s] Recieve Task(%lu)\n",
		// 					 m_workerName, (size_t)pRunnable.get());
		// 			pRunnable->Run();
		// 		}
		// 	} while ( IsBusy() );
		// }
	}
	m_exitWait.Set();
}

bool BackgroundWorker::GetFrontTask(std::shared_ptr<IRunnable> &outTask, const unsigned long long &milliSec)
{
	outTask = nullptr;
	if ( !m_inputWait.WaitOne(milliSec) )
	{
		return false;
	}

	std::lock_guard<std::mutex> _(m_cs);
	if ( !IsBusy() || m_msgQueue.empty() )
	{
		return false;
	}
	outTask = m_msgQueue.front();
	m_msgQueue.pop();
	m_numTask.fetch_sub(1);

	return true;
}

BackgroundWorker::BackgroundWorker(const char *workerName, const size_t &queueSize) :
	m_workerName(workerName),
	m_maxTask(queueSize),
	m_numTask(0),
	m_isBusy(false)
{
	DebugDetailOut("Created BackgroundWorker[%s]\n", m_workerName);
}

BackgroundWorker::~BackgroundWorker()
{
	Abort();
	DebugDetailOut("Destroyed BackgroundWorker[%s]\n", m_workerName);
}

bool BackgroundWorker::AddTask(std::shared_ptr<IRunnable> runnable)
{
	if ( nullptr == runnable )
	{
		return false;
	}

	std::lock_guard<std::mutex> _(m_cs);

	if ( m_numTask.load() >= m_maxTask )
	{
		return false;
	}

	m_msgQueue.push(runnable);
	m_numTask.fetch_add(1);
	m_inputWait.Set();

	return true;
}

void BackgroundWorker::ClearTask()
{
	std::lock_guard<std::mutex> _(m_cs);

	std::queue<std::shared_ptr<IRunnable>> emptyQueue;
	std::swap(m_msgQueue, emptyQueue);
}

bool BackgroundWorker::WaitToExit(const unsigned long long &milliSec)
{
	m_exitWait.WaitOne(milliSec);
	return !m_isBusy.load();
}

void BackgroundWorker::Run()
{
	bool expectedBusyState = false;
	if ( !std::atomic_compare_exchange_strong(&m_isBusy, &expectedBusyState, true) )
	{
		DebugOut("BackgroundWorker is running\n");
		return;
	}

	m_exitWait.Reset();
	m_worker = std::thread(&BackgroundWorker::BackgroundProc, this);
}

void BackgroundWorker::Abort()
{
	std::lock_guard<std::mutex> _(m_cs);
	m_isBusy.store(false);
	if ( m_worker.joinable() )
	{
		m_worker.detach();
		std::thread emptyWorker = std::thread();
		std::swap(m_worker, emptyWorker);
		DebugOut("BackgroundWorker[%s] Abort()\n", m_workerName);
	}
	m_exitWait.Set();
}

AutoResetEvent::AutoResetEvent(const bool &initWait)
	: m_wait(initWait)
{}

void AutoResetEvent::Set()
{
	std::lock_guard<std::mutex> _(m_cs);
	m_wait = false;
	m_signal.notify_one();
}

void AutoResetEvent::Reset()
{
	std::lock_guard<std::mutex> _(m_cs);
	m_wait = true;
}

bool AutoResetEvent::WaitOne(const unsigned long long &milliSec)
{
	if ( !m_wait )
	{
		return true;
	}

	std::unique_lock<std::mutex> lk(m_cs);
	if ( 0 == milliSec )
	{
		m_signal.wait(lk);
	} 
	else 	
	{
		auto timeout = std::chrono::duration<double, std::milli>(milliSec);
		m_signal.wait_for(lk, timeout);
	}

	return !m_wait;
}
