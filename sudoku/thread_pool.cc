#include "thread_pool.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <vector>
#include <functional>
#include <string>
#include <unistd.h>
using namespace std;
using namespace tp;


threadpool::threadpool(int max_size, const string name):max_size_(max_size), poolname_(name){}

void threadpool::run_in_thread()
{
	TASK task;
	while(running)
	{
		task  = take_task();
		if(task)
			task();
	}	
}

threadpool::~threadpool()
{
	if(running)
		stop();
}

void threadpool::start()
{
	for(int i =0; i< max_size_; i++)
		threads_.push_back(thread(bind(&threadpool::run_in_thread, this)));		
	
}

TASK threadpool::take_task()
{
	//这里当任务队列的大小大于0的时候获取任务
	TASK task;
	unique_lock<mutex> lg(mu);		//这里已经上锁了

	if(!tasks_.empty())
	{

		task = tasks_.front();
		tasks_.pop_front();
	}	
	else
	{	
		wait_task.wait(lg);			//这里如果任务为0 就一直等着直到有任务进来了把它唤醒

		if(tasks_.empty())			//这里被唤醒可能是要结束了，所以很可能没有任务
			return NULL;
		//wait需要一把独占锁。。。
		task = tasks_.front();
		tasks_.pop_front();
	}
	 return task;
}

void threadpool::stop()
{

	running = false;

	wait_task.notify_all();
	for(int i=0; i<max_size_; i++)
	{
		threads_[i].join();
	}
}



void threadpool::push_task(TASK task)
{
	do
	{
		unique_lock<mutex> ul(mu);		
		tasks_.push_back(task);
		wait_task.notify_one();			//锁住条件变量然后发出条件

	}while(0);

}

		

