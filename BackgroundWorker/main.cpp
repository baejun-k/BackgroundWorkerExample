#include <stdio.h>
#include <thread>
#include <mutex>
#include <Windows.h>

#include "BackgroundWorker.h"

class TestRun : public IRunnable {
public:

	virtual void Run() override
	{
		for ( int i = 0; i < 100; ++i )
		{
			printf("%d ", i);
		}
		printf("\n");
	}
};

int main()
{
	BackgroundWorker bgw("TestWorker");

	bgw.Run();

	for ( int i = 0; i < 10; ++i )
	{
		bgw.AddTask(std::shared_ptr<IRunnable>(new TestRun()));
	}

	if ( !bgw.WaitToExit(10) )
	{
		fprintf(stderr, "\n\nThere are %lu tasks left\n", bgw.GetNumTask());
	}

	return 0;
}