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
	/// ��ȣ�� �� ������ ��ٸ� �� �ְ� ���� ��.
	/// </summary>
	/// <param name="initWait">�������� ��ٸ��� �� ��</param>
	explicit AutoResetEvent(const bool &initWait = true);

	/// <summary>
	/// Set�� ���ָ� �ٸ� ���� Wait ���� ���� Ǯ� �����ϰ� ��.
	/// </summary>
	void Set();

	/// <summary>
	/// �ٽ� Wait ���·� �ʱ�ȭ
	/// </summary>
	void Reset();

	/// <summary>
	/// Set ��ȣ�� �ϳ� �� ������ �����.
	/// ��� �ð��� 0���� �ָ� ��ȣ�� �� ������ ������ ���
	/// </summary>
	/// <param name="milliSec">��ȣ�� �� ������ ����� �ð�</param>
	/// <returns>true:��ȣ�� ��, false:��ȣ�� ���� �� ��</returns>
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
	/// ��׶��忡�� �ݺ� ����Ǵ� ����
	/// </summary>
	void BackgroundProc();

	bool GetFrontTask(std::shared_ptr<IRunnable> &outTask, const unsigned long long &milliSec = 10);

public:
	/// <summary>
	/// Background(�񵿱�)�� �۾� ��������ش�.
	/// </summary>
	/// <param name="workerName">BackgroundWorker�� �̸�</param>
	/// <param name="queueSize">Background �۾������ �ִ� ��</param>
	BackgroundWorker(const char *workerName = "", const size_t &queueSize = 100);
	~BackgroundWorker();

	/// <summary>
	/// background worker ����.
	/// Run()�ϸ� Abort()�� ������ ������� ����.
	/// </summary>
	virtual void Run() override;
	
	/// <summary>
	/// ��׶��忡�� �۾��� ���� �߰�
	/// </summary>
	/// <param name="runnable"></param>
	/// <returns></returns>
	virtual bool AddTask(std::shared_ptr<IRunnable> runnable) override;

	/// <summary>
	/// ���� �޽��� ť�� �ִ� �۾��� ����.
	/// </summary>
	virtual void ClearTask() override final;

	/// <summary>
	/// ���� ��׶��� �۾��� �����.(���� �۾����� �Ѿ�� �ʴ´�.)
	/// �޽��� ť�� ����� ����.
	/// ���� ���� ���� IRunnable�� �������� �ʴ� ����.
	/// </summary>
	virtual void Abort() override;

	/// <summary>
	/// BackgroundWorker�� Abort()�� ����(IsBusy�� false�� ����)�� ��ٸ�
	/// </summary>
	/// <param name="milliSec">���� Ȯ�α��� ��� �ð�</param>
	/// <returns>true:�����, false:������� ����</returns>
	virtual bool WaitToExit(const unsigned long long &milliSec = 0) override;

	/// <summary>
	/// BackgroundWorker�� ���� ������
	/// </summary>
	/// <returns></returns>
	__forceinline const bool IsBusy()
	{
		return m_isBusy.load();
	}

	/// <summary>
	/// BackgroundWorker�� �ִ� �۾���� ��
	/// </summary>
	/// <returns></returns>
	__forceinline const size_t GetQueueSize()
	{
		return m_maxTask.load();
	}

	/// <summary>
	/// ���� �۾���Ͽ� �ִ� �۾� ��
	/// </summary>
	/// <returns></returns>
	__forceinline const size_t GetNumTask()
	{
		return m_numTask.load();
	}
};

#endif // !__BACKGROUND_WORKER_H__
