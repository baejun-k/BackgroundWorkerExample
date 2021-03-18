#ifndef __IASYNC_WORKER_H__
#define __IASYNC_WORKER_H__

#include <iostream>

class IRunnable 
{
public:
	virtual ~IRunnable(){}

	virtual void Run() = 0;
};


class IAsyncWorker
{
public:
	virtual ~IAsyncWorker()
	{}
	virtual bool AddTask(std::shared_ptr<IRunnable> runnable) = 0;
	virtual void ClearTask() = 0;
	virtual void Run() = 0;
	virtual void Abort() = 0;
	virtual bool WaitToExit(const unsigned long long &milliSec = 0) = 0;
};


#endif // !__IASYNC_WORKER_H__
